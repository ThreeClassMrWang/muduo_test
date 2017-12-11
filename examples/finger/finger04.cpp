//
// Created by wcj on 12/12/17.
//

#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>
#include <muduo/base/Logging.h>
#include <muduo/base/Timestamp.h>

void onMessage(const muduo::net::TcpConnectionPtr& conn, muduo::net::Buffer* buffer,
        muduo::Timestamp receiveTime ) {
    const char *clf = buffer->findCRLF();
    if (clf) {
        muduo::string message(buffer->peek(), clf);
        LOG_INFO << "On message :" << message;
        conn->shutdown();
    }
}

int main(int argc, char *argv[]) {
    muduo::net::EventLoop loop;
    muduo::net::TcpServer server(&loop, muduo::net::InetAddress(1097), "Finger04");
    server.setConnectionCallback(muduo::net::defaultConnectionCallback);
    server.setMessageCallback(onMessage);

    server.start();
    loop.loop();
}
