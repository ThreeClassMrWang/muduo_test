//
// Created by wcj on 12/25/17.
//

#include "Sock4aServer.h"

using namespace std::placeholders;

Sock4aServer::Sock4aServer(muduo::net::EventLoop *loop, const muduo::net::InetAddress &serverAddress) :
    loop_(loop), server_(loop_, serverAddress, "Sock4aServer"), threadNum_(0) {
    server_.setConnectionCallback(std::bind(&Sock4aServer::onServerConnection, this, _1));
    server_.setMessageCallback(std::bind(&Sock4aServer::onServerMessage, this, _1, _2, _3));
    server_.setWriteCompleteCallback(std::bind(&Sock4aServer::onWriteComplete, this, _1));
}

void Sock4aServer::setThreadNum(int threadNum) {
    threadNum_ = threadNum;
    server_.setThreadNum(threadNum_);
}

void Sock4aServer::start() {
    server_.start();
}

void Sock4aServer::onServerConnection(const muduo::net::TcpConnectionPtr &conn) {
    LOG_INFO << conn->peerAddress().toIpPort()
             << " -> "
             << conn->localAddress().toIpPort()
             << " is "
             << (conn->connected() ? "UP" : "DOWN");
    if (conn->connected()) {
        conn->setTcpNoDelay(true);
        conn->setHighWaterMarkCallback(std::bind(&Sock4aServer::onHighWaterMark, this, _1, _2), MAX_BUFFER);
    } else {
        // If client with target server
        if (!conn->getContext().empty()) {
            auto clientConn = boost::any_cast<std::shared_ptr<muduo::net::TcpClient>>(conn->getContext());
            if (clientConn->connection())
                clientConn->connection()->shutdown();
            clientConn.reset();
        }
    }
}

void Sock4aServer::onServerMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buff,
                                   muduo::Timestamp receiveTime) {
    LOG_INFO << conn->name()
             << " receive "
             << buff->readableBytes()
             << " bytes";
    if (conn->getContext().empty()) {
        // Not connect with each other before
        if (buff->readableBytes() > 512) {
            conn->shutdown();   // Close if get too big data
        } else if (buff->readableBytes() > 8) {
            const char* begin = buff->peek() + 8;
            const char* end = buff->peek() + buff->readableBytes();
            const char* where = std::find(begin, end, '\0');
            if (where != end) {
                char ver = buff->peek()[0];
                char cmd = buff->peek()[1];
                const void* port = buff->peek() + 2;
                const void* dest = buff->peek() + 4;

                sockaddr_in addr;
                bzero(&addr, sizeof(addr));
                addr.sin_family = AF_INET;
                addr.sin_port = *static_cast<const in_port_t*>(port);
                addr.sin_addr.s_addr = *static_cast<const uint32_t*>(dest);
                bool sock4a = muduo::net::sockets::networkToHost32(addr.sin_addr.s_addr) < 256;
                bool ok = false;
                if (sock4a) {
                    const char* endOfHostName = std::find(where+1, end, '\0');
                    if (endOfHostName != end) {
                        muduo::string hostname = where + 1;
                        where = endOfHostName;
                        LOG_INFO << "Sock4a host name " << hostname;
                        muduo::net::InetAddress tmp;
                        if (muduo::net::InetAddress::resolve(hostname, &tmp)) {
                            addr.sin_addr.s_addr = tmp.ipNetEndian();
                            ok = true;
                        }
                    } else {
                        return;;
                    }
                } else {
                    ok = true;
                }

                muduo::net::InetAddress serverAddr(addr);
                if (ver == 4 && cmd == 1 && ok) {
                    // Connect with the target server
                    conn->getLoop()->runInLoop(std::bind(&Sock4aServer::setupClientConnection, this, conn, serverAddr));
                    buff->retrieveUntil(where + 1);
                } else {
                    char response[] = "\000\0x5bUVWXYZ";
                    conn->send(response, sizeof(response));
                    conn->shutdown();
                }

            }
        }
    } else {
        const auto& clientConn = boost::any_cast<std::shared_ptr<muduo::net::TcpClient>>(conn->getContext());
        clientConn->connection()->send(buff);
    }
}

void Sock4aServer::setupClientConnection(const muduo::net::TcpConnectionPtr &serverConn,
                                         const muduo::net::InetAddress &clientAddress) {
    // Setup Client
    std::shared_ptr<muduo::net::TcpClient> clientConn(new muduo::net::TcpClient(serverConn->getLoop(), clientAddress, "Sock4aClient"));
    clientConn->setConnectionCallback(std::bind(&Sock4aServer::onClientConnection, this, _1, serverConn, clientConn, clientAddress));
    clientConn->setMessageCallback(std::bind(&Sock4aServer::onClientMessage, this, _1, _2, _3));
    clientConn->setWriteCompleteCallback(std::bind(&Sock4aServer::onWriteComplete, this, _1));
    clientConn->connect();
}

void Sock4aServer::onClientConnection(const muduo::net::TcpConnectionPtr &conn,
                                      const muduo::net::TcpConnectionPtr &serverConn,
                                      const std::shared_ptr<muduo::net::TcpClient> clientConn,
                                      const muduo::net::InetAddress& clientAddress) {
    LOG_INFO << "Sock4aClient "
             << conn->localAddress().toIpPort()
             << " -> "
             << conn->peerAddress().toIpPort()
             << " is "
             << (conn->connected() ? "UP" : "DOWN");
    if (conn->connected()) {
        conn->setTcpNoDelay(true);
        conn->setHighWaterMarkCallback(std::bind(&Sock4aServer::onHighWaterMark, this, _1, _2), MAX_BUFFER);
        serverConn->setContext(clientConn);
        clientConn->connection()->setContext(serverConn);
        // Send back to server's peer device
        char response[] = "\000\0x5aUVWXYZ";
        const sockaddr_in *addr = (const sockaddr_in*)(clientAddress.getSockAddr());
        memcpy(response+2, &addr->sin_port, 2);
        memcpy(response+4, &addr->sin_addr.s_addr, 4);
        serverConn->send(response, sizeof(response));
    } else {
        // Get server connection and shutdown
        if (!conn->getContext().empty()) {
            const auto &serverConn = boost::any_cast<muduo::net::TcpConnectionPtr>(conn->getContext());
            serverConn->shutdown();
        }
    }
}

void Sock4aServer::onClientMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buff,
                                   muduo::Timestamp receiveTime) {
    // Relay data from Server connected with socks4a client to the Client connected with sock4a server
    if (!conn->getContext().empty()) {
        const auto &serverConn = boost::any_cast<muduo::net::TcpConnectionPtr>(conn->getContext());
        serverConn->send(buff);
    } else {
        buff->retrieveAll();
        abort();
    }
}

void Sock4aServer::onHighWaterMark(const muduo::net::TcpConnectionPtr &conn, size_t bytesToRead) {
    LOG_INFO << conn->name()
             << __FUNCTION__
             << bytesToRead
             << " bytes to read, stop read";
    conn->stopRead();
}

void Sock4aServer::onWriteComplete(const muduo::net::TcpConnectionPtr &conn) {
    LOG_INFO << conn->name()
             << __FUNCTION__
             << " resume read";
    conn->startRead();
}
