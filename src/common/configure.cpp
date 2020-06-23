#include "common/configure.h"
#include "unistd.h"
#include "fcntl.h"
#include "strings.h"
#include <string>

Configure::Configure() : logger_(spdlog::get("console")) {

}

Configure& Configure::instance() {
  static Configure obj;
  return obj;
}

void Configure::load(std::string conf_path) {
  int fd = open(conf_path.c_str(), O_RDONLY);
  if(fd < 0) {
    logger_->critical("file open error, {}", strerror(errno));
    spdlog::shutdown();
    exit(-1);
  }
  off_t file_size = lseek(fd, 0, SEEK_END);
  std::string tmp;
  tmp.resize(file_size, 0);
  lseek(fd, 0, SEEK_SET);
  ssize_t n = read(fd, &(*tmp.begin()), file_size);
  if(n != file_size) {
    logger_->critical("should read {} bytes, but n == {}.", file_size, n);
    spdlog::shutdown();
    exit(-1);
  }
  content_ = json::parse(tmp);
}

