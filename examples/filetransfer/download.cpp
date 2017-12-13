//
// Created by wcj on 17-12-13.
//

#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>
#include <muduo/base/Logging.h>
#include "fileutil.h"

using namespace muduo::net;
using namespace muduo;
using namespace std::placeholders;

const char *g_file = NULL;

void onConnection(const TcpConnectionPtr& conn) {
    LOG_INFO << conn->name()
             << conn->peerAddress().toIpPort()
             << " -> "
             << conn->localAddress().toIpPort()
             << " is "
             << (conn->connected() ? "UP" : "DOWN");
    if (conn->connected()) {
        LOG_INFO << conn->name()
                 << " Sending file "
                 << g_file
                 << " to "
                 << conn->peerAddress().toIpPort();
        string fileContent = FileUtil::readFile(g_file);
        conn->send(fileContent);
        conn->shutdown();
        LOG_INFO << "Download server done";
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [file_path]\n", argv[0]);
        return EXIT_FAILURE;
    } else {
        g_file = argv[1];
        if (!(FileUtil::checkFileRead(g_file)))
            throw std::runtime_error("file do not exist or have no read access");
        EventLoop loop;
        TcpServer server(&loop, InetAddress(2021), "DownloadServer");
        server.setConnectionCallback(onConnection);
        server.start();
        loop.loop();
    }
}