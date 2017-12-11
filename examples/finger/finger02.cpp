//
// Created by wcj on 12/8/17.
//

#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

/// Accept new connection, but do nothing after connected.
/// Abandon new message from peer device.
///
/// \param argc
/// \param argv
/// \return
int main(int argc, char *argv[]) {
    muduo::net::EventLoop loop;
    muduo::net::TcpServer server(&loop, muduo::net::InetAddress(1079), "Finger");
    server.start();
    loop.loop();
}
