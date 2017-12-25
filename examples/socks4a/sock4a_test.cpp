//
// Created by wcj on 12/26/17.
//

#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpClient.h>
#include <muduo/base/Logging.h>

using namespace std::placeholders;
using namespace muduo::net;
using namespace muduo;

static int count = 0;

static void onConnection(const TcpConnectionPtr& conn) {
    LOG_INFO << conn->name()
             << conn->localAddress().toIpPort()
             << " -> "
             << conn->peerAddress().toIpPort()
             << " is "
             << (conn->connected() ? "UP" : "DOWN");
    if (conn->connected()) {
        conn->setTcpNoDelay(true);
        char response[] = "\004\001UVWXYZhello\000127.0.0.1";
        uint16_t *port = (uint16_t *) (response + 2);
        uint32_t *ip = (uint32_t *) (response + 4);
        *port = sockets::hostToNetwork16(10007);
        *ip = sockets::hostToNetwork32(1);
        conn->send(response, sizeof(response));
        LOG_INFO << "send reponse "
                 << response;
    }
}

static void onMessage(const TcpConnectionPtr& conn, Buffer* buff, Timestamp receiveTime) {
    LOG_INFO << "Receive message "
             << string(buff->peek());

    if (count < 10)
        conn->send("Hello");

    count++;
}

/// TODO NEED MORE TEST（基本可以跑通，但是不知道是否存在内存泄露的情况）
///
/// \param argc
/// \param argv
/// \return
int main(int argc, char* argv[]) {
    EventLoop loop;
    TcpClient client(&loop, muduo::net::InetAddress(9999), "sock4aTestClient");
    client.setConnectionCallback(std::bind(onConnection, _1));
    client.setMessageCallback(std::bind(onMessage, _1, _2, _3));

    client.connect();
    loop.loop();
}