#ifndef CHAT_INCLUDE_REDIS_REDIS_H
#define CHAT_INCLUDE_REDIS_REDIS_H

#include <hiredis/hiredis.h>

#include <string>
#include <functional>

// 典型的观察者模式
class Redis {
public:
  Redis();
  ~Redis();
  bool connect(const char* ip = "127.0.0.1", int port=6379);
  bool publish(int channel, std::string message);
  bool subscribe(int channel);
  bool unsubscribe(int channel);
  
  void observeChannelMessage();
  void setNotifyHandler(std::function<void(int, std::string)> fn);

private:

  redisContext* publishCtx_;  // 发布者上下文
  redisContext* subscribeCtx_;  // 订阅者上下文
  std::function<void(int, std::string)> handler_;

};



#endif  // CHAT_INCLUDE_REDIS_REDIS_H