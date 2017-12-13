//
// Created by wcj on 12/13/17.
//

#include <muduo/net/EventLoop.h>
#include <muduo/base/ThreadPool.h>
#include <muduo/net/TcpServer.h>
#include <muduo/base/Timestamp.h>
#include <muduo/base/ThreadPool.h>
#include <muduo/base/Logging.h>
#include "sudoku.h"

using namespace std::placeholders;

class SudokuServer {
public:
    SudokuServer(uint16_t port, muduo::string name, int workThreadNum, int computeThreadNum) :
            loop_(), server_(&loop_, muduo::net::InetAddress(port), name),
            threadPool_("Sudoku Solver"),
            workThreadNum_(workThreadNum), computeThreadNum_(computeThreadNum){
        server_.setConnectionCallback(muduo::net::defaultConnectionCallback);
        server_.setMessageCallback(std::bind(&SudokuServer::onMessage, this, _1, _2, _3));
        server_.setThreadNum(workThreadNum_);
        threadPool_.setMaxQueueSize(computeThreadNum_);
    }

    void start() {
        threadPool_.start(computeThreadNum_);
        server_.start();
        loop_.loop();
    }

private:
    muduo::net::EventLoop loop_;
    muduo::net::TcpServer server_;
    muduo::ThreadPool threadPool_;
    int workThreadNum_;
    int computeThreadNum_;
    static constexpr size_t MAX_DATA_LEN = 4096;

    void onMessage(const muduo::net::TcpConnectionPtr &conn,
        muduo::net::Buffer *buff, muduo::Timestamp receiveTime) {
        size_t len = buff->readableBytes();
        while (len >= static_cast<size_t >(kCells) + 2) {
            const char *crlf = buff->findCRLF();

            // Big illegal data
            if (!crlf && len >= MAX_DATA_LEN) {
                LOG_INFO << "Big illegal data from "
                         << conn->peerAddress().toIpPort();
                conn->shutdown();
            }

            if (crlf) {
                muduo::string request(buff->peek(), crlf);
                muduo::string id;
                buff->retrieveUntil(crlf + 2);
                auto colon = std::find(request.begin(), request.end(), ':');
                if (colon != request.end()) {   // Find id
                    id.assign(request.begin(), colon);
                    request.erase(request.begin(), colon + 1);
                }

                // Correct data length
                if (request.size() == static_cast<size_t >(kCells)) {
                    threadPool_.run(std::bind(&SudokuServer::resolve, conn, request, id));
                } else {
                    LOG_INFO << "Bad format from "
                             << conn->peerAddress().toIpPort();
                    conn->send("Bad request!\r\n");
                    conn->shutdown();
                }

            } else {
                // Data bot complete, just break and wait
                break;
            }

        }
    }

    static void resolve(const muduo::net::TcpConnectionPtr& conn,
        const muduo::string& puzzle, const muduo::string& id) {
        LOG_INFO << "resove from "
                 << conn->peerAddress().toIpPort();

        muduo::string result = solveSudoku(puzzle);
        // Have no big use, just in case
        if (conn->connected()) {
            if (id.empty())
                conn->send(result + "\r\n");
            else
                conn->send(id + ":" + result + "\r\n");
        }
    }
};


int main(int argc, char *argv[]) {
    SudokuServer server(9981, "SudokuServer", 3, 3);
    server.start();
}