//
// Created by wcj on 17-12-12.
//

#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>
#include <muduo/base/Timestamp.h>
#include <muduo/base/Logging.h>
#include "sudoku.h"

#define MAX_DATA_LEN        1024

void onMessage(const muduo::net::TcpConnectionPtr& conn, muduo::net::Buffer *buff,
    muduo::Timestamp receiveTime) {
    LOG_DEBUG << conn->name();

    size_t len = buff->readableBytes();

    while (len >= kCells + 2) {
        const char *crlf = buff->findCRLF();
        // Illegal big request.
        if (!crlf && len >= MAX_DATA_LEN) {
            LOG_INFO << "Illegal big request from "
                     << conn->peerAddress().toIpPort();
            conn->shutdown();
        }

        // Find a request
        if (crlf) {
            muduo::string request(buff->peek(), crlf);
            muduo::string id;
            buff->retrieveUntil(crlf + 2);
            auto colon = std::find(request.begin(), request.end(), ':');
            // Find id
            if (colon != request.end()) {
                id.assign(request.begin(), colon);
                request.erase(request.begin(), colon + 1);
            }

            if (request.size() == static_cast<size_t >(kCells)) {
                muduo::string result = solveSudoku(request);
                if (id.empty())
                    conn->send(result + "\r\n");
                else
                    conn->send(id + ":" + result + "\r\n");
                LOG_INFO << "Sudoku result :"
                         << id << " : " << result;
            } else {
                // Illegal request
                LOG_INFO << "Illegal request, bad format from "
                         << conn->peerAddress().toIpPort();
                conn->send("Bad request!\r\n");
                conn->shutdown();
            }
        } else {
            // Request not complete
            break;
        }
    }
}

int main(int argc, char *argv[]) {
    muduo::net::EventLoop loop;
    muduo::net::TcpServer server(&loop, muduo::net::InetAddress(9981), "Sudoku");
    server.setConnectionCallback(muduo::net::defaultConnectionCallback);
    server.setMessageCallback(onMessage);

    server.start();
    loop.loop();
}