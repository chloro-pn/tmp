#include "sserver/storage_server.h"
#include "sserver/storage_server_context.h"
#include "json.hpp"
#include "sserver/block_file.h"
#include "common/md5_info.h"
#include "common/message.h"
#include "md5.h"
#include <vector>

using nlohmann::json;

void StorageServer::onConnection(std::shared_ptr<TcpConnection> con) {
  logger_->trace("storage server {} connect.", con->iport());
  con->set_context(std::make_shared<StorageServerContext>());
  StorageServerContext* state = con->get_context<StorageServerContext>();
  if(state->init() == false) {
    logger_->error("path storage init fail. {}", con->iport());
    con->force_close();
    return;
  }
  state->setState(StorageServerContext::state::transfering_block_set);
  state->transferingMd5s() = state->pathStorage().getAllItems();
  sendSomeMd5sToProxy(con);
}

void StorageServer::onMessage(std::shared_ptr<TcpConnection> con) {
  logger_->trace("on message.");
  StorageServerContext* context = con->get_context<StorageServerContext>();
  json j = json::parse(std::string(con->message_data(), con->message_length()));

  if(Message::getType(j) == "upload_block") {
    Md5Info md5 = Message::getMd5FromUploadBlockMessage(j);
    //check if the same md5 block is uploading or have been stored in disk.
    //#######bug.#######
    std::string content = Message::getContentFromUploadBlockMessage(j);
    context->uploadingMd5s()[md5].append(content);
    if(Message::theLastBlockPiece(j) == true) {
      MD5 tmp(context->uploadingMd5s()[md5]);
      if(tmp.toStr() != md5.getMd5Value()) {
        context->uploadingMd5s().erase(md5);
        std::string fail_message = Message::constructUploadBlockFailMessage(md5);
        logger_->warn("upload block fail. {} != {}", md5.getMd5Value(), tmp.toStr());
        con->send(fail_message);
        context->uploadingMd5s().erase(md5);
      }
      else {
        bool exist = context->pathStorage().checkItem(md5);
        if(exist == true) {
          std::string ack_message = Message::constructUploadBlockAckMessage(md5);
          logger_->trace("block {} exist.send ack.", md5.getMd5Value());
          con->send(ack_message);
          context->uploadingMd5s().erase(md5);
        }
        else {
          BlockFile bf;
          bool succ = bf.createNewFile(context->getBlockFilePath(md5));
          if(succ == false) {
            logger_->critical("create new file error.");
            con->force_close();
            return;
          }
          bf.writeBlock(context->uploadingMd5s()[md5]);
          context->pathStorage().storageItemPath(md5, context->getBlockFilePath(md5));
          std::string ack_message = Message::constructUploadBlockAckMessage(md5);
          logger_->trace("upload block ack. {}", md5.getMd5Value());
          con->send(ack_message);
          context->uploadingMd5s().erase(md5);
        }
      }
    }
  }
  else {
    logger_->error("error state.");
    con->force_close();
    return;
  }
}

void StorageServer::onWriteComplete(std::shared_ptr<TcpConnection> con) {
  logger_->trace("on write complete.");
  StorageServerContext* context = con->get_context<StorageServerContext>();
  if(context->getState() == StorageServerContext::state::transfering_block_set) {
    sendSomeMd5sToProxy(con);
    logger_->trace("continue to send block md5 sets.");
  }
  else if(context->getState() == StorageServerContext::state::working) {
    //continue sending block.
    //if current block send over.
  }
  else {
    logger_->error("error state.");
    con->force_close();
    return;
  }
}

void StorageServer::onClose(std::shared_ptr<TcpConnection> con) {
  StorageServerContext* context = con->get_context<StorageServerContext>();
  for(auto& each : context->uploadingMd5s()) {
    std::string fail_message = Message::constructUploadBlockFailMessage(each.first);
    con->send(fail_message);
  }
  logger_->warn("on close, state : {}", con->get_state_str());
}

StorageServer::StorageServer(asio::io_context& io, std::string ip, std::string port, std::shared_ptr<spdlog::logger> logger):
                             client_(io, ip, port),
                             logger_(logger) {
  client_.setOnConnection([this](std::shared_ptr<TcpConnection> con)->void {
    this->onConnection(con);
  });

  client_.setOnMessage([this](std::shared_ptr<TcpConnection> con)->void {
    this->onMessage(con);
  });

  client_.setOnWriteComplete([this](std::shared_ptr<TcpConnection> con)->void {
    this->onWriteComplete(con);
  });

  client_.setOnClose([this](std::shared_ptr<TcpConnection> con)->void {
    this->onClose(con);
  });
  logger_->trace("storage server start.");
}

void StorageServer::connectToProxyServer() {
  logger_->trace("connect to proxy.");
  client_.connect();
}

void StorageServer::sendSomeMd5sToProxy(std::shared_ptr<TcpConnection> con) {
  StorageServerContext* state = con->get_context<StorageServerContext>();
  std::vector<Md5Info> now_trans;
  for(;state->nextToTransferMd5Index() < 1024; ++(state->nextToTransferMd5Index())) {
    if(state->nextToTransferMd5Index() >= state->transferingMd5s().size()) {
      break;
    }
    now_trans.push_back(state->transferingMd5s()[state->nextToTransferMd5Index()]);
  }
  std::string message;
  if(state->nextToTransferMd5Index() >= state->transferingMd5s().size()) {
    message = Message::constructTransferBlockSetMessage(now_trans, true);
    state->nextToTransferMd5Index() = 0;
    state->transferingMd5s().clear();
    state->setState(StorageServerContext::state::working);
  }
  else {
    message = Message::constructTransferBlockSetMessage(now_trans, false);
  }
  con->send(std::move(message));
}