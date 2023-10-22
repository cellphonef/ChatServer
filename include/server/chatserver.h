#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <mymuduo/TcpServer.h>
#include <mymuduo/EventLoop.h>
#include <mymuduo/Timestamp.h>

#include <string>

class ChatServer {
 public:
  ChatServer(mymuduo::EventLoop* loop, const mymuduo::InetAddress& listenAddr,
             const std::string& nameArg);

  void start();

 private:
  void onConnection(const mymuduo::TcpConnectionPtr& conn);
  void onMessage(const mymuduo::TcpConnectionPtr& conn, mymuduo::Buffer* buf,
                 mymuduo::Timestamp time);

  mymuduo::TcpServer server_;
  mymuduo::EventLoop* loop_;
};

#endif  // CHATSERVER_H