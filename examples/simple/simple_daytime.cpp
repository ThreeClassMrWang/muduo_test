//
// Created by wcj on 17-12-13.
//

#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>
#include <muduo/base/Logging.h>

using namespace std::placeholders;
using namespace muduo;
using namespace muduo::net;

class DaytimeServer :noncopyable {
public:
    DaytimeServer(uint16_t port, const string& name) :
            loop_(), address_(port), server_(&loop_, address_, name) {
        server_.setConnectionCallback(std::bind(&DaytimeServer::onConnection,
                                             this, _1));
    }

    void start() {
        server_.start();
        loop_.loop();
    }

private:
    EventLoop loop_;
    InetAddress address_;
    TcpServer server_;

    void onConnection(const TcpConnectionPtr& conn) {
        LOG_INFO << conn->name()
                 << " got daytime request from "
                 << conn->peerAddress().toIpPort()
                 << (conn->connected() ? " UP" : " Down");
        if (conn->connected()) {
            conn->send(Timestamp::now().toFormattedString() + "\r\n");
            conn->shutdown();
        }
    }
};

int main(int argc, char *argv[]) {
    DaytimeServer server(10013, "DaytimeServer");
    server.start();
}
