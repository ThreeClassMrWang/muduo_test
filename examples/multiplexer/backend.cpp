//
// Created by wcj on 12/22/17.
//

#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>
#include <muduo/base/Logging.h>

using namespace muduo::net;
using namespace muduo;
using namespace std::placeholders;

class BackendServer : muduo::noncopyable {
public:
    BackendServer(EventLoop* loop, InetAddress address) :
            loop_(loop), server_(loop_, address, "BackendServer") {
        server_.setConnectionCallback(std::bind(&BackendServer::onConnection, this, _1));
        server_.setMessageCallback(std::bind(&BackendServer::onMessage, this, _1, _2, _3));
    }

    void start() {
        server_.start();
    }

private:
    EventLoop* loop_;
    TcpServer server_;
    static constexpr int MAX_PACKET_LEN = 512;
    static constexpr int HEADER_LEN = 8;

    enum ClientType {
        CONN = 0,
        DISCONN,
        MSG,
    };

    void onConnection(const TcpConnectionPtr& conn) {
        LOG_INFO << "BackendServer : "
                 << conn->peerAddress().toIpPort()
                 << " -> "
                 << conn->localAddress().toIpPort()
                 << " is "
                 << (conn->connected() ? "UP" : "DOWN");
    }

    void onMessage(const TcpConnectionPtr& conn, Buffer* buff, Timestamp reveiveTime) {
        while (buff->readableBytes() > HEADER_LEN + 1) {
            int32_t len = buff->peekInt32();
            if (len < 0 || len > MAX_PACKET_LEN) {
                LOG_ERROR << conn->name()
                          << "received invalid data";
                conn->shutdown();
            }

            if (buff->readableBytes() < HEADER_LEN + 1 + len) {
                break;
            } else {
                uint32_t *idbe32 = (uint32_t *)(buff->peek() + sizeof(int32_t));
                int32_t id = sockets::networkToHost32(*idbe32);
                char type = *(buff->peek() + HEADER_LEN);
                string msg(buff->peek() + HEADER_LEN + 1, len);
                onStringMessage(id, type, msg);
                buff->retrieve(HEADER_LEN + 1 + len);
            }

        }
    }

    void onStringMessage(int32_t id, char type, const string& msg) {
        if (type == CONN) {
            LOG_INFO << id
                     << " connect "
                     << msg;
        } else if (type == DISCONN) {
            LOG_INFO << id
                     << " disconnect "
                     << msg;
        } else {
            LOG_INFO << id
                     << " message "
                     << msg;
        }
    }
};

int main(int argc, char* argv[]) {
    EventLoop loop;
    InetAddress listenAddress(9999);
    BackendServer server(&loop, listenAddress);
    server.start();
    loop.loop();
}
