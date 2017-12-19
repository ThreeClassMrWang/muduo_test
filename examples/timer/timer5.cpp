//
// Created by wcj on 12/18/17.
//

#include <muduo/net/EventLoop.h>
#include <muduo/net/EventLoopThread.h>
#include <iostream>
#include <set>

class Printer {
public:
    using EventLoopList = std::set<muduo::net::EventLoop*>;
    Printer(const EventLoopList& list) : list_(list), count_(0) {
        for (const auto& loop : list_)
            loop->runEvery(1, std::bind(&Printer::print, this, loop));
    }
    ~Printer() {
        std::cout << "Good bye~" << std::endl;
    }

    void print(muduo::net::EventLoop* loop) {
        bool quit = false;
        {
            muduo::MutexLockGuard lock(mutex_);
            if (count_ < 10) {
                std::cout << "Hello World! [" << count_ << "]" << std::endl;
                ++count_;
            } else {
                quit = true;
            }
        }

        if (quit) loop->quit();
    }

private:
    const EventLoopList& list_;
    int count_;
    muduo::MutexLock mutex_;
};

int main(int argc, char* argv[]) {
    muduo::net::EventLoop loop1;
    muduo::net::EventLoopThread loop2;
    Printer::EventLoopList loopList;
    loopList.insert(&loop1);
    loopList.insert(loop2.startLoop());

    std::shared_ptr<Printer> printerPtr(new Printer(loopList));
    loop1.loop();
}

