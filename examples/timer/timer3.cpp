//
// Created by wcj on 12/18/17.
//

#include <muduo/net/EventLoop.h>
#include <iostream>
#include <functional>

static void print(muduo::net::EventLoop* loop, int *count) {
    if (*count < 5) {
        std::cout << "Hello [" << *count << "]" << std::endl;
        ++(*count);
    } else {
        std::cout << "Good byte~" << std::endl;
        loop->quit();
    }
}

int main(int argc, char* argv[]) {
    muduo::net::EventLoop loop;
    int count = 0;
    loop.runEvery(1.0, std::bind(print, &loop, &count));
    loop.loop();
}