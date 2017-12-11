//
// Created by wcj on 12/8/17.
//

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpConnection.h>

static void onConnection(const muduo::net::TcpConnectionPtr &conn) {
    if (conn->connected())
        conn->shutdown();
}

/// Disconnect if connected with peer device.
///
/// \param argc
/// \param argv
/// \return

int main(int argc, char *argv[]) {
    muduo::net::EventLoop loop;
    muduo::net::TcpServer server(&loop, muduo::net::InetAddress(1079), "Finger03");
    server.setConnectionCallback(onConnection);
    server.start();
    loop.loop();
}
