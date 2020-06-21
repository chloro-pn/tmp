#ifndef MESSAGE_H
#define MESSAGE_H

#include "md5_info.h"
#include "json.hpp"
#include <string>
#include <vector>

using nlohmann::json;

namespace Message {
//all type
std::string getType(const json& j);

//UPLOAD_REQUEST
std::vector<Md5Info> getMd5FromUploadRequestMessage(const json& j);

std::string createUploadRequestMessage(const std::vector<Md5Info>& md5s);

//UPLOAD_RESPONSE
std::string createUploadResponseMessage(const std::vector<Md5Info>& md5s);

std::vector<Md5Info> getMd5FromUploadResponseMessage(const json& j);

//UPLOAD_BLOCK
std::string createUploadBlockMessage(const Md5Info& md5, uint32_t index, bool eof, std::string&& content);

bool theFirstBlockPiece(const json& j);

bool theLastBlockPiece(const json& j);

Md5Info getMd5FromUploadBlockMessage(const json& j);

std::string getContentFromUploadBlockMessage(const json& j);

//UPLOAD_BLOCK_ACK
std::string constructUploadBlockAckMessage(const Md5Info& md5);

Md5Info getMd5FromUploadBlockAckMessage(const json& j);

//UPLOAD_BLOCK_FAIL
std::string constructUploadBlockFailMessage(const Md5Info& md5);

Md5Info getMd5FromUploadBlockFailMessage(const json& j);

//UPLOAD_ALL_BLOCKS
std::string constructUploadAllBlocksMessage();

//FILE_STORE_SUCC
std::string constructFileStoreSuccMessage(Md5Info file_id);

//FILE_STORE_FAIL
std::string constructFileStoreFailMessage(Md5Info file_id);

//TRANSFER_BLOCK_SET
std::vector<Md5Info> getMd5sFromTransferBlockSetMessage(const json& j);

std::string constructTransferBlockSetMessage(const std::vector<Md5Info>& md5s, bool eof);

bool theLastTransferBlockSet(const json& j);
}

#endif // MESSAGE_H