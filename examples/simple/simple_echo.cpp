//
// Created by wcj on 17-12-13.
//

#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>
#include <muduo/base/Logging.h>
#include <muduo/base/noncopyable.h>

using namespace std::placeholders;

class EchoServer : muduo::noncopyable {
public:
    EchoServer(uint16_t port, const muduo::string &name) :
            loop_(), serverAddr_(port),
            server_(&loop_, serverAddr_, name) {
        server_.setConnectionCallback(muduo::net::defaultConnectionCallback);
        server_.setMessageCallback(std::bind(&EchoServer::onMessage, this, _1, _2, _3));
    }

    void start() {
        server_.start();
        loop_.loop();
    }

private:
    muduo::net::EventLoop loop_;
    muduo::net::InetAddress serverAddr_;
    muduo::net::TcpServer server_;

    void onMessage(const muduo::net::TcpConnectionPtr& conn,
        muduo::net::Buffer* buff, muduo::Timestamp receiveTime){
        muduo::string msg(buff->retrieveAllAsString());
        LOG_INFO << "got message ["
                 << msg << "] from "
                 << conn->peerAddress().toIpPort();
        conn->send(msg);
    }
};

int main(int argc, char *argv[]) {
    EchoServer server(10007, "EchoServer");
    server.start();
}