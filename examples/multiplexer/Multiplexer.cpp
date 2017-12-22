//
// Created by wcj on 12/22/17.
//

#include "Multiplexer.h"

using namespace std::placeholders;

Multiplexer::Multiplexer(muduo::net::EventLoop *loop, const muduo::net::InetAddress &listenAddr,
                         const muduo::net::InetAddress &backendAddr, int numThreads) :
    loop_(loop),
    server_(loop, listenAddr, "MultiplexerServer"),
    backend_(loop, backendAddr, "MultiplexerBackend"),
    numThread_(numThreads), oldCounter_(0), startTime_(muduo::Timestamp::now()){
    server_.setConnectionCallback(std::bind(&Multiplexer::onClientConnection, this, _1));
    server_.setMessageCallback(std::bind(&Multiplexer::onClientMessage, this, _1, _2, _3));

    backend_.setConnectionCallback(std::bind(&Multiplexer::onBackendConnection, this, _1));
    backend_.setMessageCallback(std::bind(&Multiplexer::onBackendMessage, this, _1, _2, _3));

    loop->runEvery(30, std::bind(&Multiplexer::printStatis, this));
}

void Multiplexer::onClientConnection(const muduo::net::TcpConnectionPtr &conn) {
    LOG_INFO << "Client "
             << conn->peerAddress().toIpPort()
             << " -> "
             << conn->localAddress().toIpPort()
             << " is "
             << (conn->connected() ? "UP" : "DOWN");
    if (conn->connected()) {
        conn->setTcpNoDelay(true);

        int32_t id = -1;
        {
            muduo::MutexLockGuard lock(mutex_);
            if (!availIds_.empty()) {
                id = availIds_.front();
                availIds_.pop();
                clientConnections_[id] = conn;
            }
        }

        if (id < 0) {
            LOG_INFO << conn->name()
                     << "have no enough connection descriptor, shutdown";
            conn->shutdown();
        } else {
            conn->setContext(id);
            // Notify backend that new Connection is UP
            char buf[256];
            snprintf(buf, sizeof(buf), "CONN %d FROM %s IS UP\r\n", id, conn->peerAddress().toIpPort().c_str());
            sendBackendString(id, ClientType::CONN, buf);
        }

    } else {
        // Remove connection
        if (!conn->getContext().empty()) {
            auto id = boost::any_cast<int32_t>(conn->getContext());
            assert(id >= 0 && id < MAX_CONNS);
            char buf[256];
            snprintf(buf, sizeof(buf), "CONN %d FROM %s is DOWN\r\n", id, conn->peerAddress().toIpPort().c_str());
            sendBackendString(id, ClientType::DISCONN, buf);

            // Give back connection descriptor
            {
                muduo::MutexLockGuard lock(mutex_);
                availIds_.push(id);
                clientConnections_.erase(id);
            }
        }

    }
}

void Multiplexer::onClientMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buff,
                                  muduo::Timestamp receiveTime) {
    size_t len = buff->readableBytes();
    transferred_.addAndGet(len);
    receivedMessage_.incrementAndGet();
    if (!conn->getContext().empty()) {
        auto id = boost::any_cast<int>(conn->getContext());
        while (buff->readableBytes() >= MAX_PACKET_LEN) {
            muduo::net::Buffer packet;
            char type = ClientType::MSG;
            packet.append(&type, sizeof(type));
            packet.append(buff->peek(), MAX_PACKET_LEN);
            buff->retrieve(MAX_PACKET_LEN);
            sendBackendPacket(id, &packet);
        }

        if (buff->readableBytes() > 0) {
            size_t len = buff->readableBytes();
            muduo::net::Buffer packet;
            char type = ClientType::MSG;
            packet.append(&type, sizeof(type));
            packet.append(buff->peek(), len);
            buff->retrieve(len);
            sendBackendPacket(id, &packet);
        }

    } else {
        buff->retrieveAll();
    }
}

void Multiplexer::sendBackendString(int32_t id, char type, const muduo::string &msg) {
    muduo::net::Buffer buf;
    char typeData = type;
    buf.append(&typeData, sizeof(typeData));
    buf.append(msg);
    sendBackendPacket(id, &buf);
}

void Multiplexer::sendBackendPacket(int32_t id, muduo::net::Buffer *buff) {
    size_t len = buff->readableBytes();
    assert(len <= MAX_PACKET_LEN);
    uint8_t header[HEADER_LEN];
    uint32_t* lenbe32 = (uint32_t *)header;
    uint32_t* idbe32 = (uint32_t *)(header + sizeof(int32_t));
    *lenbe32 = muduo::net::sockets::hostToNetwork32((uint32_t)len - 1);
    *idbe32 = muduo::net::sockets::hostToNetwork32((uint32_t)id);
    buff->prepend(header, HEADER_LEN);
    muduo::net::TcpConnectionPtr conn;
    {
        muduo::MutexLockGuard lock(mutex_);
        conn = backendConnection_;
    }

    if (conn)
        conn->send(buff);
}

void Multiplexer::onBackendConnection(const muduo::net::TcpConnectionPtr &conn) {
    LOG_INFO << "Backend "
             << conn->localAddress().toIpPort()
             << " -> "
             << conn->peerAddress().toIpPort()
             << " is "
             << (conn->connected() ? "UP" : "DOWN");
    std::vector<muduo::net::TcpConnectionPtr> connsToDestory;
    if (conn->connected()) {
        conn->setTcpNoDelay(true);
        muduo::MutexLockGuard lock(mutex_);
        backendConnection_ = conn;
        assert(availIds_.empty());
        for (int32_t i = 0; i < MAX_CONNS; ++i)
            availIds_.push(i);
    } else {
        {
            muduo::MutexLockGuard lock(mutex_);
            backendConnection_.reset();
            connsToDestory.reserve(clientConnections_.size());
            for (const auto &clientConn : clientConnections_)
                connsToDestory.push_back(clientConn.second);
            clientConnections_.clear();
            while (!availIds_.empty()) availIds_.pop();
        }
           // Shutdown
        for (auto &clientConn : connsToDestory)
            clientConn->shutdown();
        loop_->quit();
    }
}

void Multiplexer::onBackendMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buff,
                                   muduo::Timestamp receiveTime) {
    size_t len = buff->readableBytes();
    transferred_.addAndGet(len);
    receivedMessage_.incrementAndGet();
    sendClientPacket(buff);
}

void Multiplexer::sendClientPacket(muduo::net::Buffer *buff) {
    while (buff->readableBytes() > HEADER_LEN + 1) {
        // Data len
        int32_t len = buff->peekInt32();
        if (buff->readableBytes() < len + HEADER_LEN + 1) {
            break;
        } else {
            int32_t *idbe32 = (int32_t *)(buff->peek() + sizeof(int32_t));
            int32_t id = muduo::net::sockets::networkToHost32(static_cast<uint32_t>(*idbe32));
            char type = *(buff->peek() + HEADER_LEN);
            muduo::string msg(buff->peek() + HEADER_LEN + 1, len);
            sendClientString(id, type, msg);
            buff->retrieve(static_cast<size_t>(HEADER_LEN + len + 1));
        }
    }
}

void Multiplexer::sendClientString(int32_t id, char type, const muduo::string &msg) {
    if (type == MSG) {
        // Find connection
        muduo::net::TcpConnectionPtr clientConn;
        {
            muduo::MutexLockGuard lock(mutex_);
            auto it = clientConnections_.find(id);
            if (it != clientConnections_.end())
                clientConn = it->second;
        }

        if (clientConn) {
            clientConn->send(msg);
        }
    } else if (type == DISCONN) {
        muduo::MutexLockGuard lock(mutex_);
        auto it = clientConnections_.find(id);
            if (it != clientConnections_.end()) {
                it->second->shutdown();
                clientConnections_.erase(it);
                availIds_.push(id);
            }
    } else {
        LOG_ERROR << "received discard message to "
                  << id;
    }
}

void Multiplexer::printStatis() {
    muduo::Timestamp nowTime = muduo::Timestamp::now();
    int64_t newCounter = transferred_.get();
    int64_t bytes = newCounter - oldCounter_;
    int64_t msgs = receivedMessage_.getAndSet(0);
    double time = muduo::timeDifference(nowTime, startTime_);
    printf("%4.3f MiB/s %4.3f Ki Msgs/s %6.2f bytes per msg\n",
           static_cast<double>(bytes)/time/1024/1024,
           static_cast<double>(msgs)/time/1024,
           static_cast<double>(bytes)/ static_cast<double>(msgs));
    oldCounter_ = newCounter;
    startTime_ = nowTime;
}

void Multiplexer::start() {
    backend_.connect();
    server_.start();
}
