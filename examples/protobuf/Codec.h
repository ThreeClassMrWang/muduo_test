//
// Created by wcj on 12/17/17.
//

#ifndef MUDUO_TEST_CODEC_H
#define MUDUO_TEST_CODEC_H

#include <functional>
#include <muduo/net/TcpServer.h>
#include "query.pb.h"

/// Protobuf protocol
///
/// | -------------- |
/// |                | len : Length of frame, do not include itself
/// | -------------- |
/// |                | namelen : Length of message name >= 2
/// | -------------- |
/// |                | message name : namelen bytes, end with '\0'
/// | -------------- |
/// |                | protobuf data : (len - namelen - 8) bytes
/// | -------------- |
/// |                | check sum: adler32 of above (namelen, typename, protobuf data).
/// | -------------- |
///
/// c struct code is:
/// \code
/// struct ProtobufTransportFormat __attribute__((__packed__)) {
///     int32_t len;
///     int32_t nameLen;
///     char typeName[nameLen];
///     char protobufData[len-nameLen-8];
///     int32_t checkSum;
/// }
///
class Codec {
public:

    enum ErrorCode {
        kNoError = 0,
        kInvalidLength,
        kCheckSumError,
        kInvalidNameLen,
        kUnknowMessageType,
        kParseError,
    };

    using MessagePtr = std::shared_ptr<google::protobuf::Message>;
    using MessageCallback = std::function<void(const muduo::net::TcpConnectionPtr&,
            const MessagePtr&, muduo::Timestamp)>;
    using ErrorCallback = std::function<void(const muduo::net::TcpConnectionPtr&,
            muduo::net::Buffer*, muduo::Timestamp, ErrorCode)>;

    Codec(const MessageCallback& cb,
          const ErrorCallback& = Codec::defaultErrorCallback) :
            messageCallback_(cb) {}

    void onMessage(const muduo::net::TcpConnectionPtr& conn,
            muduo::net::Buffer* buff, muduo::Timestamp receiveTime);
    void send(const muduo::net::TcpConnectionPtr& conn,
            const google::protobuf::Message& msg) {
        muduo::net::Buffer buff;
        fillEmptyBuffer(&buff, msg);
        conn->send(&buff);
    }

    static const muduo::string& errorCodeToString(ErrorCode errorCode);
    static void fillEmptyBuffer(muduo::net::Buffer* buff, const google::protobuf::Message& message);
    static google::protobuf::Message* createMessage(const std::string& typeName);
    static MessagePtr parse(const char* buf, int len, ErrorCode* errorCode);

private:
    MessageCallback messageCallback_;
    ErrorCallback errorCallback_;
    static void defaultErrorCallback(const muduo::net::TcpConnectionPtr& conn,
        muduo::net::Buffer* buff, muduo::Timestamp receiveTime, ErrorCode errorCode);

    static constexpr int kHeaderLen = sizeof(int32_t);
    static constexpr int kMinMessageLen = 2*kHeaderLen + 2;
    static constexpr int kMaxMessageLen = 64*1024*1024;
};


#endif //MUDUO_TEST_CODEC_H
