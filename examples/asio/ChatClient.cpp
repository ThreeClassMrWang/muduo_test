//
// Created by wcj on 17-12-14.
//

#include <muduo/base/Logging.h>
#include "ChatClient.h"

using namespace std::placeholders;

ChatClient::ChatClient(muduo::net::EventLoop *loop, const muduo::net::InetAddress &address) :
    loop_(loop), client_(loop_, address, "ChatClient"),
    codec_(std::bind(&ChatClient::onStringMessage, this, _1, _2, _3)){
    client_.setConnectionCallback(std::bind(&ChatClient::onConnection, this, _1));
    client_.setMessageCallback(std::bind(&Codec::onMessage, &codec_, _1, _2, _3));
}

void ChatClient::connect() {
    client_.connect();
}

void ChatClient::disconnect() {
    client_.disconnect();
}

void ChatClient::write(const muduo::StringPiece &message) {
    muduo::net::TcpConnectionPtr tempConn;
    {
        muduo::MutexLockGuard lock(mutex_);
        tempConn = connection_;
    }

    if (tempConn) {
        codec_.send(tempConn, message);
    }
}

void ChatClient::onConnection(const muduo::net::TcpConnectionPtr &conn) {
    LOG_INFO << conn->name()
             << " "
             << conn->localAddress().toIpPort()
             << " -> "
             << conn->peerAddress().toIpPort()
             << " is "
             << (conn->connected() ? "UP" : "DOWN");
    if (conn->connected()) {
        conn->setTcpNoDelay(true);
        muduo::MutexLockGuard lock(mutex_);
        connection_ = conn;
    } else {
        muduo::MutexLockGuard lock(mutex_);
        connection_.reset();
    }
}

void ChatClient::onStringMessage(const muduo::net::TcpConnectionPtr &conn, const muduo::string &message,
                                 muduo::Timestamp receiveTime) {
    printf("<<< %s\n", static_cast<const char*>(&message[0]));
}
