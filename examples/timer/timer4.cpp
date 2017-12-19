//
// Created by wcj on 12/18/17.
//

#include <muduo/net/EventLoop.h>
#include <muduo/base/noncopyable.h>
#include <iostream>

class Printer : muduo::noncopyable {
public:
    Printer(muduo::net::EventLoop* loop) : loop_(loop), count_(0) {
        loop_->runEvery(1, std::bind(&Printer::print, this));
    }
    ~Printer() {
        std::cout << "Good bye~" << std::endl;
    }

    void print(void) {
        if (count_ < 5) {
            std::cout << "Hello World [" << count_ << "]" << std::endl;
            ++count_;
        } else {
            loop_->quit();
        }
    }

private:
    muduo::net::EventLoop* loop_;
    int count_;
};

int main(int argc, char* argv[]) {
    muduo::net::EventLoop loop;
    Printer printer(&loop);
    loop.loop();
}
