//
// Created by wcj on 17-12-14.
//

#include <muduo/net/EventLoopThread.h>
#include <muduo/net/TcpClient.h>
#include <iostream>
#include "ChatClient.h"

using namespace muduo::net;
using namespace muduo;

int main(int argc, char *argv[]) {
    EventLoopThread loopThread;
    InetAddress serverAddress("0.0.0.0",10000);

    ChatClient client(loopThread.startLoop(), serverAddress);
    client.connect();

    std::string line;
    while(std::getline(std::cin, line))
        client.write(line);
    client.disconnect();
}