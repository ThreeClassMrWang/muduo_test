//
// Created by wcj on 17-12-14.
//

#ifndef MUDUO_TEST_CODEC_H
#define MUDUO_TEST_CODEC_H

#include <functional>
#include <muduo/net/TcpServer.h>

class Codec : muduo::noncopyable {
public:
    using StringMessageCallback = std::function<void (const muduo::net::TcpConnectionPtr& conn,
            const muduo::string& message, muduo::Timestamp receiveTime)>;

    explicit Codec(const StringMessageCallback& cb) :
            messageCallback_(cb) { }

    void onMessage(const muduo::net::TcpConnectionPtr& conn,
            muduo::net::Buffer* buff, muduo::Timestamp receiveTime);
    void send(const muduo::net::TcpConnectionPtr& conn,
            const muduo::StringPiece& message);

private:
    StringMessageCallback messageCallback_;
    static constexpr size_t HEADER_LEN = 4;
};

#endif //MUDUO_TEST_CODEC_H
