//
// Created by wcj on 12/22/17.
//

#ifndef MUDUO_TEST_MULTIPLEXER_H
#define MUDUO_TEST_MULTIPLEXER_H

#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/net/TcpServer.h>
#include <muduo/net/TcpClient.h>
#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include <muduo/base/Atomic.h>
#include <queue>

class Multiplexer : muduo::noncopyable {
public:
    Multiplexer(muduo::net::EventLoop* loop, const muduo::net::InetAddress& listenAddr,
        const muduo::net::InetAddress& backendAddr, int numThreads);

    void start();

private:
    muduo::net::EventLoop* loop_;
    muduo::net::TcpServer server_;
    muduo::net::TcpClient backend_;
    int numThread_;
    muduo::AtomicInt64 transferred_;
    muduo::AtomicInt64 receivedMessage_;
    int64_t oldCounter_;
    muduo::Timestamp startTime_;

    muduo::MutexLock mutex_;
    muduo::net::TcpConnectionPtr backendConnection_;
    std::map<int32_t, muduo::net::TcpConnectionPtr> clientConnections_;
    std::queue<int32_t> availIds_;
    static constexpr int MAX_CONNS = 65535;
    static constexpr int MAX_PACKET_LEN = 512;
    static constexpr int HEADER_LEN = 8;

    enum ClientType {
        CONN = 0,
        DISCONN,
        MSG,
    };

    void onClientConnection(const muduo::net::TcpConnectionPtr& conn);
    void onClientMessage(const muduo::net::TcpConnectionPtr& conn,
        muduo::net::Buffer* buff, muduo::Timestamp receiveTime);
    void sendClientPacket(muduo::net::Buffer* buff);
    void sendClientString(int32_t id, char type, const muduo::string& msg);

    void onBackendConnection(const muduo::net::TcpConnectionPtr& conn);
    void onBackendMessage(const muduo::net::TcpConnectionPtr& conn,
        muduo::net::Buffer* buff, muduo::Timestamp receiveTime);
    void sendBackendString(int32_t id, char type, const muduo::string& msg);
    void sendBackendPacket(int32_t id, muduo::net::Buffer* buff);

    void printStatis();


};


#endif //MUDUO_TEST_MULTIPLEXER_H
