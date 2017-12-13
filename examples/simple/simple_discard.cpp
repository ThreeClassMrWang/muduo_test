//
// Created by wcj on 17-12-13.
//

#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>
#include <muduo/base/Logging.h>

using namespace muduo::net;
using namespace muduo;
using namespace std::placeholders;

class DiscardServer :noncopyable {
public:
    DiscardServer(uint16_t port, const string& name) :
            loop_(), sevaddr_(port), server_(&loop_, sevaddr_, name) {
        server_.setConnectionCallback(defaultConnectionCallback);
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

    void onMessage(const TcpConnectionPtr& conn,
        Buffer* buff, Timestamp receiveTime) {
        string msg(buff->retrieveAllAsString());
        LOG_INFO << conn->name()
                 << " discards "
                 << msg.size()
                 << " bytes received at "
                 << conn->peerAddress().toIpPort();
    }
};

int main(int argc, char *argv[]) {
    DiscardServer server(10009, "DiscardServer");
    server.start();
}
