//
// Created by wcj on 17-12-13.
//

#include <muduo/net/TcpClient.h>
#include <muduo/net/EventLoop.h>
#include <muduo/base/Logging.h>
#include <muduo/base/noncopyable.h>

using namespace std::placeholders;
using namespace muduo::net;
using namespace muduo;

class TimeClient : noncopyable {
public:
    TimeClient(const InetAddress& srvAddr, const string& name) :
            loop_(), client_(&loop_, srvAddr, name) {
        client_.setConnectionCallback(std::bind(&TimeClient::onConnection, this, _1));
        client_.setMessageCallback(std::bind(&TimeClient::onMessage, this, _1, _2, _3));
    }

    void start() {
        client_.connect();
        loop_.loop();
    }

private:
    EventLoop loop_;
    TcpClient client_;

    void onConnection(const TcpConnectionPtr& conn) {
        LOG_INFO << conn->localAddress().toIpPort()
                 << " -> "
                 << conn->peerAddress().toIpPort()
                 << " is "
                 << (conn->connected() ? "UP" : "DOWN");
        if (!conn->connected())
            loop_.quit();
    }

    void onMessage(const TcpConnectionPtr& conn, Buffer *buff,
        Timestamp receiveTime) {
        if (buff->readableBytes() >= sizeof(uint64_t)) {
            const void *data = buff->peek();
            uint64_t be64 = *static_cast<const uint64_t *>(data);
            buff->retrieve(sizeof(uint64_t));
            time_t t = sockets::networkToHost64(be64);
            Timestamp ts(t * Timestamp::kMicroSecondsPerSecond);
            LOG_INFO << "Server time is "
                     << t
                     << " ,"
                     << ts.toFormattedString();
            conn->shutdown();
        }
    }
};

int main(int argc, char *argv[]) {
    if (argc != 2) {
        LOG_ERROR << "Usage : "
                  << argv[0]
                  << " [host_ip]";
        return EXIT_SUCCESS;
    } else {
        InetAddress address(argv[1], 10007);
        TimeClient client(address, "TimeClient");
        client.start();
    }
}