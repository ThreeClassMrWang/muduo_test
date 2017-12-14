//
// Created by wcj on 17-12-14.
//

#ifndef MUDUO_TEST_CHATCLIENT_H
#define MUDUO_TEST_CHATCLIENT_H


#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpClient.h>
#include <muduo/base/Mutex.h>
#include "Codec.h"

class ChatClient {
public:
    ChatClient(muduo::net::EventLoop* loop, const muduo::net::InetAddress& address);

    void connect();
    void disconnect();

    void write(const muduo::StringPiece& message);

private:
    muduo::net::EventLoop* loop_;
    muduo::net::TcpClient client_;
    Codec codec_;
    muduo::MutexLock mutex_;
    muduo::net::TcpConnectionPtr connection_;

    void onConnection(const muduo::net::TcpConnectionPtr& conn);
    void onStringMessage(const muduo::net::TcpConnectionPtr& conn,
            const muduo::string& message, muduo::Timestamp receiveTime);

};


#endif //MUDUO_TEST_CHATCLIENT_H
