//
// Created by wcj on 12/14/17.
//

#ifndef MUDUO_TEST_CHATSERVERHP_H
#define MUDUO_TEST_CHATSERVERHP_H

#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>
#include <muduo/base/ThreadLocalSingleton.h>
#include <set>
#include "Codec.h"

class ChatServerHp {
public:
    ChatServerHp(muduo::net::EventLoop* loop, const muduo::net::InetAddress& address);

    void start(void);
    void setThreadNum(int threadNum);

private:
    muduo::net::EventLoop* loop_;
    muduo::net::TcpServer server_;
    Codec codec_;
    muduo::MutexLock mutex_;
    std::set<muduo::net::EventLoop*> loops_;
    using ConnectionList = std::set<muduo::net::TcpConnectionPtr>;
    using LocalConnection = muduo::ThreadLocalSingleton<ConnectionList>;

    void onThreadInit(muduo::net::EventLoop* loop);
    void onConnection(const muduo::net::TcpConnectionPtr& conn);
    void onStringMessage(const muduo::net::TcpConnectionPtr& conn,
                         const muduo::string& message,
                         muduo::Timestamp receiveTime);
    void onDistributeMessage(const muduo::string& message);
};


#endif //MUDUO_TEST_CHATSERVERHP_H
