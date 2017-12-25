//
// Created by wcj on 12/25/17.
//

#ifndef MUDUO_TEST_SOCK4ASERVER_H
#define MUDUO_TEST_SOCK4ASERVER_H

#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>
#include <muduo/net/TcpClient.h>
#include <muduo/base/Logging.h>

class Sock4aServer : muduo::noncopyable {
public:
    Sock4aServer(muduo::net::EventLoop* loop,
                 const muduo::net::InetAddress& serverAddress);

    void setThreadNum(int threadNum);
    void start();

private:
    muduo::net::EventLoop* loop_;
    muduo::net::TcpServer server_;
    int threadNum_;
    static constexpr size_t MAX_BUFFER = 10 * 1024 * 1024;  /* 10MB */

    void onServerConnection(const muduo::net::TcpConnectionPtr& conn);
    void onServerMessage(const muduo::net::TcpConnectionPtr& conn,
        muduo::net::Buffer* buff, muduo::Timestamp receiveTime);


    void onClientConnection(const muduo::net::TcpConnectionPtr& conn,
                            const muduo::net::TcpConnectionPtr &serverConn,
                            const std::shared_ptr<muduo::net::TcpClient> clientConn,
                            const muduo::net::InetAddress& clientAddress);
    void onClientMessage(const muduo::net::TcpConnectionPtr& conn,
        muduo::net::Buffer* buff, muduo::Timestamp receiveTime);

    void onHighWaterMark(const muduo::net::TcpConnectionPtr& conn,
        size_t bytesToRead);
    void onWriteComplete(const muduo::net::TcpConnectionPtr& conn);

    void setupClientConnection(const muduo::net::TcpConnectionPtr& serverConn,
        const muduo::net::InetAddress& clientAddress);

};


#endif //MUDUO_TEST_SOCK4ASERVER_H
