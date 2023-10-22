#include "include/server/chatserver.h"
#include "include/server/chatservice.h"
#include "thirdparty/json.h"

#include <functional>
#include <string>

using namespace std;
using namespace std::placeholders;
using json = nlohmann::json;

ChatServer::ChatServer(mymuduo::EventLoop* loop,
                       const mymuduo::InetAddress& listenAddr,
                       const std::string& nameArg)
    : server_(loop, listenAddr, nameArg), loop_(loop) {
  server_.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));
  server_.setMessageCallback(
      std::bind(&ChatServer::onMessage, this, _1, _2, _3));
  server_.setThreadNum(4);
}

void ChatServer::start() { server_.start(); }

void ChatServer::onConnection(const mymuduo::TcpConnectionPtr& conn) {
  if (!conn->connected()) {  // 连接关闭
    ChatService::getInstance().clientCloseException(conn);
  }
}

void ChatServer::onMessage(const mymuduo::TcpConnectionPtr& conn,
                           mymuduo::Buffer* buf, mymuduo::Timestamp time) {
  string message = buf->retrieveAllAsString();
  json js = json::parse(message);
  // 解耦网络模块以及业务模块
  auto handler = ChatService::getInstance().getHandler(js["msgId"].get<int>());
  handler(conn, js, time);
}