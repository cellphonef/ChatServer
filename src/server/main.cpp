#include "include/server/chatserver.h"
#include "include/server/chatservice.h"
#include <mymuduo/EventLoop.h>
#include <mymuduo/InetAddress.h>

#include <signal.h>

// 服务器Cttl+C退出
void resetHandler(int) {
  ChatService::getInstance().reset();
  exit(1);
}

// 服务器其他方式退出？



int main(int argc, char* argv[]) {
    struct sigaction sa;
    sa.sa_handler = resetHandler;
    sigaction(SIGINT, &sa, nullptr);

    mymuduo::EventLoop loop;
    mymuduo::InetAddress listenAddr(atoi(argv[1]));
    ChatServer server(&loop, listenAddr, "ChatServer");

    server.start();
    loop.loop();

    return 0;
}