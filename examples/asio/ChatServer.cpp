//
// Created by wcj on 17-12-14.
//

#include <muduo/base/Mutex.h>
#include "ChatServer.h"

using namespace std::placeholders;
using namespace muduo::net;
using namespace muduo;

ChatServer::ChatServer(muduo::net::EventLoop *loop, const muduo::net::InetAddress &addr) :
    loop_(loop), server_(loop_, addr, "ChatServer"),
    codec_(std::bind(&ChatServer::onStringMessage, this, _1, _2, _3)){
    server_.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));
    server_.setMessageCallback(std::bind(&Codec::onMessage, &codec_, _1, _2, _3));
}

void ChatServer::onConnection(const muduo::net::TcpConnectionPtr &conn) {
    LOG_INFO << conn->localAddress().toIpPort()
             << " -> "
             << conn->peerAddress().toIpPort()
             << " is "
             << (conn->connected() ? "UP" : "DOWN");
    if (conn->connected()) {
        conn->setTcpNoDelay(true);
        MutexLockGuard lock(mutex_);
        connections_.insert(conn);
    } else {
        MutexLockGuard lock(mutex_);
        connections_.erase(conn);
    }
}

void ChatServer::onStringMessage(const muduo::net::TcpConnectionPtr &conn, const muduo::string& message,
                                 muduo::Timestamp receiveTime) {
    // Copy on write, to minimize critical area
    ConnectionSet tempConnections;
    {
        MutexLockGuard lock(mutex_);
        tempConnections = connections_;
    }

    for (const auto& conn : tempConnections) {
        codec_.send(conn, message);
    }
}

void ChatServer::start() {
    server_.start();
}

void ChatServer::setThreadNum(int threadNum) {
    server_.setThreadNum(threadNum);
}
