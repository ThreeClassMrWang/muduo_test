//
// Created by wcj on 17-12-13.
//

#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>
#include <muduo/base/Logging.h>
#include <muduo/net/Endian.h>
#include <time.h>

using namespace std::placeholders;
using namespace muduo::net;
using namespace muduo;

class TimeServer : noncopyable{
public:
    TimeServer(uint16_t port, const string& name) :
            loop_(), address_(port), server_(&loop_, address_, name) {
        server_.setConnectionCallback(std::bind(&TimeServer::onConnection, this, _1));
    }

    void start() {
        server_.start();
        loop_.loop();
    };

private:
    EventLoop loop_;
    InetAddress address_;
    TcpServer server_;

    void onConnection(const TcpConnectionPtr& conn) {
        LOG_INFO << conn->name()
                 << " got connection from "
                 << conn->peerAddress().toIpPort()
                 << (conn->connected() ? " UP" : " Down");
        if (conn->connected()) {
            time_t now = time(NULL);
            uint64_t be64 = sockets::hostToNetwork64(static_cast<uint64_t>(now));
            conn->send(&be64, sizeof(be64));
            conn->shutdown();
        }
    }
};

int main(int argc, char *argv[]) {
    TimeServer server(10007, "TimeServer");
    server.start();
}