//
// Created by wcj on 17-12-14.
//

#include "ChatServer.h"

using namespace muduo;
using namespace muduo::net;

int main(int argc, char *argv[]) {
    EventLoop loop;
    InetAddress address(10000);
    ChatServer server(&loop, address);
    server.start();
    loop.loop();
}