//
// Created by wcj on 12/25/17.
//

#include "Sock4aServer.h"

int main(int argc, char* argv[]) {
    muduo::net::EventLoop loop;
    muduo::net::InetAddress serverAddress(9999);
    Sock4aServer server(&loop, serverAddress);

    server.setThreadNum(4);
    server.start();
    loop.loop();
}