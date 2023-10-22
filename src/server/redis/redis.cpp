#include "include/server/redis/redis.h"

#include <iostream>
#include <thread>

Redis::Redis() : publishCtx_(nullptr), subscribeCtx_(nullptr) {}

Redis::~Redis() {
  redisFree(publishCtx_);
  redisFree(subscribeCtx_);
}

// 与Redis服务器建立连接
bool Redis::connect(const char* ip, int port) {
  // 建立连接
  publishCtx_ = redisConnect(ip, port);
  if (publishCtx_ == nullptr) {
    std::cerr << "redisConnect failed!" << std::endl;
    return false;
  }
  subscribeCtx_ = redisConnect(ip, port);
  if (subscribeCtx_ == nullptr) {
    std::cerr << "redisConnect failed!" << std::endl;
    return false;
  }

  std::thread t([&]() { observeChannelMessage(); });
  t.detach();
  
  std::cout << "redisConnect success!" << std::endl;

  return true;
}

// 向channel通道发布消息
bool Redis::publish(int channel, std::string message) {
  redisReply* reply = (redisReply*)redisCommand(publishCtx_, "PUBLISH %d %s",
                                                channel, message.c_str());
  if (reply == nullptr) {
    std::cerr << "publish command failed!" << std::endl;
    return false;
  }
  freeReplyObject(reply);
  std::cout << "publish command success!" << std::endl;
  return true;
}

// 向channel订阅
bool Redis::subscribe(int channel) {
  if (REDIS_ERR == redisAppendCommand(subscribeCtx_, "SUBSCRIBE %d", channel)) {
    std::cerr << "subscribe command failed!" << std::endl;
    return false;
  }

  int done = 0;
  while (!done) {
    if (REDIS_ERR == redisBufferWrite(subscribeCtx_, &done)) {
      std::cerr << "subscribe command failed!" << std::endl;
      return false;
    }
  }

  std::cout << "subscribe command success!" << std::endl;
  return true;
}

// 解channel订阅
bool Redis::unsubscribe(int channel) {
  if (REDIS_ERR == redisAppendCommand(subscribeCtx_, "UNSUBSCRIBE %d", channel)) {
    std::cerr << "unsubscribe command failed!" << std::endl;
    return false;
  }

  int done = 0;
  while (!done) {
    if (REDIS_ERR == redisBufferWrite(subscribeCtx_, &done)) {
      std::cerr << "unsubscribe command failed!" << std::endl;
      return false;
    }
  }

  std::cout << "unsubsricbe command success!" << std::endl;
  return true;
}

// 依次处理订阅消息
void Redis::observeChannelMessage() {
  redisReply* reply = nullptr;
  while (REDIS_OK == redisGetReply(subscribeCtx_, (void**)&reply)) {
    if (reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr) {
        handler_(atoi(reply->element[1]->str), reply->element[2]->str);
    }
    freeReplyObject(reply);
  }
  std::cout << "observerChannelMessage success!" << std::endl;
}

void Redis::setNotifyHandler(std::function<void(int, std::string)> fn) {
  handler_ = fn;
}
