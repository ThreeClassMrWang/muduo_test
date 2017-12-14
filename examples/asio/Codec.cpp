//
// Created by wcj on 17-12-14.
//

#include <muduo/base/Logging.h>
#include "Codec.h"

void Codec::send(const muduo::net::TcpConnectionPtr &conn,
                 const muduo::StringPiece &message) {
    muduo::net::Buffer buff;
    buff.append(message.data(), static_cast<size_t>(message.size()));
    uint32_t len = static_cast<uint32_t>(message.size());
    uint32_t be32 = muduo::net::sockets::hostToNetwork32(len);
    buff.prepend(&be32, sizeof(be32));
    conn->send(&buff);
}

void Codec::onMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buff,
                      muduo::Timestamp receiveTime) {
    while (buff->readableBytes() >= HEADER_LEN) {
        uint32_t len = static_cast<uint32_t>(buff->peekInt32());
        if (len > UINT16_MAX || len < 0) {
            LOG_ERROR << "Invalid length "
                      << len;
            conn->forceClose(); // ? conn->shutdown()
            break;
        } else if (buff->readableBytes() >= len + HEADER_LEN) {
            buff->retrieve(HEADER_LEN);
            muduo::string msg(buff->peek(), len);
            if (messageCallback_) messageCallback_(conn, msg, receiveTime);
            buff->retrieve(static_cast<size_t>(len));
        } else {
            break;
        }
    }
}
