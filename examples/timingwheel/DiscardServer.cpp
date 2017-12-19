#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/base/noncopyable.h>
#include <muduo/base/Logging.h>
#include "TimingWheel.h"

using namespace muduo::net;
using namespace muduo;
using namespace std::placeholders;

class DiscardServer :noncopyable {
public:
    DiscardServer(uint16_t port, const string& name) :
            loop_(), sevaddr_(port), server_(&loop_, sevaddr_, name) ,
            timeWheel_(&loop_, 8) {
        server_.setConnectionCallback(std::bind(&DiscardServer::onConnection, this, _1));
        server_.setMessageCallback(std::bind(&DiscardServer::onMessage,
                                             this, _1, _2, _3));
    }

    void start() {
        server_.start();
        loop_.loop();
    }

private:
    EventLoop loop_;
    InetAddress sevaddr_;
    TcpServer server_;
    TimingWheel timeWheel_;
    

    void onMessage(const TcpConnectionPtr& conn,
        Buffer* buff, Timestamp receiveTime) {
        string msg(buff->retrieveAllAsString());
        LOG_INFO << conn->name()
                 << " discards "
                 << msg.size()
                 << " bytes received at "
                 << conn->peerAddress().toIpPort();
        timeWheel_.update(conn);
    }

    void onConnection(const TcpConnectionPtr& conn) {
        LOG_INFO << conn->name() << conn->peerAddress().toIpPort()
                 << " -> " << conn->localAddress().toIpPort() << " is "
                 << (conn->connected() ? "UP" : "DOWN");

        if (conn->connected()) {
            timeWheel_.add(conn);
        }
    }
};

int main(int argc, char* arcv[]) {
    DiscardServer server(10000, "DiscardServer");
    server.start();
}