//
// Created by wcj on 12/14/17.
//

#include <functional>
#include <muduo/base/Logging.h>
#include "ChatServerHp.h"

using namespace std::placeholders;

ChatServerHp::ChatServerHp(muduo::net::EventLoop *loop, const muduo::net::InetAddress &address) :
    loop_(loop), server_(loop_, address, "ChatServerHp"),
    codec_(std::bind(&ChatServerHp::onStringMessage, this, _1, _2, _3)) {
    server_.setConnectionCallback(std::bind(&ChatServerHp::onConnection, this, _1));
    server_.setMessageCallback(std::bind(&Codec::onMessage, &codec_, _1, _2, _3));
    server_.setThreadInitCallback(std::bind(&ChatServerHp::onThreadInit, this, _1));
}

void ChatServerHp::start() {
    server_.start();
}

void ChatServerHp::setThreadNum(int threadNum) {
    server_.setThreadNum(threadNum);
}

void ChatServerHp::onThreadInit(muduo::net::EventLoop *loop) {
    LocalConnection::instance();
    muduo::MutexLockGuard lock(mutex_);
    loops_.insert(loop);
}

void ChatServerHp::onConnection(const muduo::net::TcpConnectionPtr &conn) {
    LOG_INFO << conn->name()
             << conn->peerAddress().toIpPort()
             << " -> "
             << conn->localAddress().toIpPort()
             << " is "
             << (conn->connected() ? "UP" : "DOWN");
    if (conn->connected()) {
        LocalConnection::instance().insert(conn);
    } else {
        LocalConnection::instance().erase(conn);
    }
}

void ChatServerHp::onStringMessage(const muduo::net::TcpConnectionPtr &conn, const muduo::string &message,
                                   muduo::Timestamp receiveTime) {
    LOG_DEBUG << conn->name()
              << " on StringMessage";
    for (auto const& loop : loops_)
        loop->runInLoop(std::bind(&ChatServerHp::onDistributeMessage, this, message));
}

void ChatServerHp::onDistributeMessage(const muduo::string &message) {
    ConnectionList  conns = LocalConnection::instance();
    for (auto const& conn : conns)
        codec_.send(conn, message);
}
