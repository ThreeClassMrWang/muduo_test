//
// Created by wcj on 12/12/17.
//

#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>
#include <muduo/base/Logging.h>
#include <muduo/base/Timestamp.h>
#include <map>

typedef std::map<muduo::string, muduo::string> UserMap;
UserMap users;

muduo::string getUser(const muduo::string& user) {
    muduo::string res = "No such user";
    auto it = users.find(user);
    if (it != users.end()) {
        res = it->second;
    }
    return res;
}

void onMessage(const muduo::net::TcpConnectionPtr& conn,
                muduo::net::Buffer* buff, muduo::Timestamp receiveTime) {
    const char *clf = buff->findCRLF();
    if (clf) {
        muduo::string user(buff->peek(), clf);
        conn->send(getUser(user) + "\r\n");
        buff->retrieveUntil(clf + 2);
        conn->shutdown();
    }
}

int main(int argc, char *argv[]) {
    users["wcj"] = "Happy and well";
    muduo::net::EventLoop loop;
    muduo::net::TcpServer server(&loop, muduo::net::InetAddress(1097), "Finger05");
    server.setConnectionCallback(muduo::net::defaultConnectionCallback);
    server.setMessageCallback(onMessage);

    server.start();
    loop.loop();
}
