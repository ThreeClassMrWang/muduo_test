//
// Created by wcj on 12/14/17.
//

#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>
#include <muduo/base/Logging.h>
#include <unistd.h>
#include <memory>
#include "fileutil.h"

using namespace std::placeholders;
using namespace muduo::net;
using namespace muduo;

typedef std::shared_ptr<FILE> FilePtr;

constexpr int kSize = 1024;    // 64kb one time
const char* g_file;

static void onHighWaterMark(const TcpConnectionPtr& conn, size_t size);

/// Use shared_ptr and RAII to manage file
///
/// \param conn
static void onConnection(const TcpConnectionPtr& conn) {
    LOG_INFO << conn->name()
             << " DownloadServer - "
             << conn->peerAddress().toIpPort()
             << " -> "
             << conn->localAddress().toIpPort()
             << " is "
             << (conn->connected() ? "UP" : "DOWN");
    if (conn->connected()) {
        LOG_INFO << "DownloadServer - send file "
                 << g_file
                 << " to "
                 << conn->peerAddress().toIpPort();
        conn->setHighWaterMarkCallback(onHighWaterMark, kSize + 1);

        FILE* fp = fopen(g_file, "r");
        if (fp == NULL) {
            LOG_ERROR << "Open file "
                      << g_file
                      << " error";
            conn->shutdown();
            return;
        }

        char buf[kSize];
        size_t nread = ::fread(buf, 1, sizeof(buf), fp);
        FilePtr filePtr(fp, ::fclose);
        conn->setContext(filePtr);
        conn->send(buf, (int)nread);
    }
}

static void onMessage(const TcpConnectionPtr& conn, Buffer* buff,
        Timestamp receiveTime) {
    string msg(buff->retrieveAllAsString());
    LOG_INFO << "Discard "
             << msg.size()
             << " bytes from "
             << conn->peerAddress().toIpPort();
}

static void onWriteComplete(const TcpConnectionPtr& conn) {
    FilePtr filePtr = boost::any_cast<FilePtr>(conn->getContext());
    assert(filePtr.get());
    char buf[kSize];
    size_t nread = fread(buf, 1, sizeof(buf), filePtr.get());
    if (nread > 0) {
        conn->send(buf, (int) nread);
    }
    else {
        conn->shutdown();
        LOG_INFO << "Send file "
                 << g_file
                 << " done!";
    }
}

static void onHighWaterMark(const TcpConnectionPtr& conn, size_t size) {
    LOG_INFO << conn->name()
             << " high water mark "
             << size;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [file_path]\n", argv[0]);
        return EXIT_FAILURE;
    } else {
        g_file = argv[1];
        if (!FileUtil::checkFileRead(g_file))
            throw std::runtime_error("file do not exist or can not be read");

        EventLoop loop;
        TcpServer server(&loop, InetAddress(2021), "DownloadServer");
        server.setConnectionCallback(onConnection);
        server.setMessageCallback(onMessage);
        server.setWriteCompleteCallback(onWriteComplete);

        server.start();
        loop.loop();
    }
}

