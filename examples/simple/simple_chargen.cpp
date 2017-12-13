//
// Created by wcj on 17-12-13.
//

#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>
#include <muduo/base/Logging.h>

using namespace std::placeholders;
using namespace muduo;
using namespace muduo::net;

class ChargenServer :noncopyable {
public:
    ChargenServer(uint16_t port, const string& name) :
            loop_(), address_(port),
            server_(&loop_, address_, name),
            msgTranffered_(0) {
        fillMsg();
        server_.setMessageCallback(std::bind(&ChargenServer::onMessage, this, _1, _2, _3));
        server_.setConnectionCallback(std::bind(&ChargenServer::onConnection, this, _1));
        server_.setWriteCompleteCallback(std::bind(&ChargenServer::onWriteComplete, this, _1));
    }

    void start() {
        server_.start();
        loop_.loop();
    }

private:
    EventLoop loop_;
    InetAddress address_;
    TcpServer server_;

    static constexpr size_t CHAR_LEN = 40;
    static constexpr size_t LINE_LEN = 50;
    static constexpr char START_CHAR = '#';
    std::vector<string> message_;
    size_t msgTranffered_;

    void fillMsg() {
        for (size_t i = 0; i < LINE_LEN; ++i) {
            string msg(CHAR_LEN, ' ');
            for (size_t j = 0; j < CHAR_LEN; ++j) {
                msg[j] = static_cast<char>(START_CHAR + i + j);
            }
            message_.push_back(msg);
        }
    }

    void onMessage(const TcpConnectionPtr& conn, Buffer *buff, Timestamp receiveTime) {
        string msg(buff->retrieveAllAsString());
        LOG_INFO << conn->name()
                 << " discards "
                 << msg.size()
                 << " bytes received from "
                 << conn->peerAddress().toIpPort();
    }

    void onConnection(const TcpConnectionPtr& conn) {
        // Close nagal algorithm
        // Avoid small piece of data was delay
        conn->setTcpNoDelay(true);
        LOG_INFO << "ChargenServer - "
                 << conn->peerAddress().toIpPort()
                 << " -> "
                 << conn->localAddress().toIpPort()
                 << " is "
                 << (conn->connected() ? "UP" : "DOWN");
        if (conn->connected()) {
            conn->send(message_[msgTranffered_++] + "\r\n");
        }
    }

    void onWriteComplete(const TcpConnectionPtr& conn) {
        if (msgTranffered_ == LINE_LEN) {
            conn->shutdown();
        } else {
            conn->send(message_[msgTranffered_++] + "\r\n");
        }
    }

};

int main(int argc, char *argv[]) {
    ChargenServer server(10019, "ChargenServer");
    server.start();
}