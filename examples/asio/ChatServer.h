//
// Created by wcj on 17-12-14.
//

#ifndef MUDUO_TEST_CHATSERVER_H
#define MUDUO_TEST_CHATSERVER_H

#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>
#include <muduo/base/Logging.h>
#include <set>
#include "Codec.h"

class ChatServer : muduo::noncopyable {
public:
    ChatServer(muduo::net::EventLoop* loop, const muduo::net::InetAddress& addr);

    void start();

    void setThreadNum(int threadNum);

private:
    muduo::net::EventLoop* loop_;
    muduo::net::TcpServer server_;
    Codec codec_;
    using ConnectionSet = std::set<muduo::net::TcpConnectionPtr>;
    ConnectionSet connections_;
    muduo::MutexLock mutex_;

    void onStringMessage(const muduo::net::TcpConnectionPtr& conn,
        const muduo::string& message, muduo::Timestamp receiveTime);

    void onConnection(const muduo::net::TcpConnectionPtr& conn);

};


#endif //MUDUO_TEST_CHATSERVER_H
