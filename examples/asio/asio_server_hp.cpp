//
// Created by wcj on 12/14/17.
//

#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>
#include <muduo/base/Logging.h>
#include "ChatServerHp.h"

int main(int argc, char* argv[]) {
    muduo::net::EventLoop loop;
    muduo::net::InetAddress address(10000);
    ChatServerHp server(&loop, address);
    server.setThreadNum(4);
    server.start();
    loop.loop();
}
