## TODO：

1.client上传文件，位置拼接可能有bug。[done]

2.MD5值的统一，字符串而不是二进制。[done]

3.json包不能发二进制数据，不是utf8字节会产生异常。

4.完成选择合适的存储服务器函数。[done]

5.proxy和sserver周期性的通信

6.可能发生向同一个sserver发送同一个md5值的block的piece，这样会导致
记录混乱，考虑加上flow号区分不同块的piece，丢弃不需要的，或者
新命令（优化）。[doing]

2020-06-23 : 在uploadblock消息中添加了flow_id字段，类型为uint64_t，客户端只要将该字段置为0即可，proxy设置该字段
并将该消息转发给存储端，因为在并发情况下可能会有多个客户端向同一个存储服务器上传同一个文件块（md5值相同），
因此存储端需要一种机制区分上传同一个文件块的不同客户端。例如：

```c++
std::map<Md5Info, std::pair<uint64_t, std::string>> uploading_blocks_;
...
Md5Info md5 = Message::getMd5FromUploadBlockMessage(message);
uint64_t flow_id = Message::getFlowIdFromUploadBlockMessage(message);
if(uploading_blocks_.find(md5) == uploading_blocks_.end()) {
  ...//log and exit.
}
else {
  auto& tmp = uploading_blocks_[md5];
  if(flow_id != tmp.first) {
    ...//不是同一个客户端上传的md5块，直接丢掉（后续可以添加流程控制消息，告诉proxy该flow_id标识的客户端不需要继续上传该块，已经有其他客户端正在上传了。）
  }
  else {
    ...//storage.
  }
}
```

这个问题在之后的文件下载功能也会出现，可能有不同的客户端同一时间向同一个存储服务器请求下载同一个块，存储服务器要根据flow_id分别
记录每个客户端当前的下载上下文。

2020-06-21 : 

由于json库不能传递二进制数据，因此消息包中的md5都采用字符串模式而不是二进制模式。

注：MD5库也许有bug，寻找新的md5计算库测试，如有问题及时替换

2020-06-22 : 
添加makefile和脚本文件用于测试。

在根目录下运行run_example.sh脚本文件，会在在examples目录下会生成三个可执行文件，并启动执行，三个可执行文件分别代表客户端，代理端和存储端。
这个示例程序会模拟一次文件上传，并生成日志文件存储在*.txt文件中。

本项目使用第三方库redis，hiredis，spdlog，json，asio需要在src/makefile和examples/makefile中根据本机情况配置头文件路径和库文件路径。
redis作为服务程序启动，端口号默认，asio和spdlog是header-only库，json我直接放到项目中了，不用配置。

注：下一步预计开配置文件接口，开始的时候为了方便很多都写死在程序中了，现在调试很不方便。[doing]
