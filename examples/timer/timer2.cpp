//
// Created by wcj on 12/18/17.
//

#include <muduo/net/EventLoop.h>
#include <iostream>

using namespace std::placeholders;

/// Print 'Hello World!' and quit.
///
/// \param loop
static void print(muduo::net::EventLoop* loop) {
    std::cout << "Hello World!" << std::endl;
    loop->quit();
}

int main(int argc, char *argv[]) {
    muduo::net::EventLoop loop;
    loop.runAfter(5, std::bind(print, &loop));
    loop.loop();
}

