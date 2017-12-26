//
// Created by wcj on 12/26/17.
//

#include <muduo/net/http/HttpServer.h>
#include <muduo/net/http/HttpResponse.h>
#include <muduo/net/http/HttpRequest.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/EventLoopThreadPool.h>
#include <muduo/base/Logging.h>
#include <map>
#include <sys/socket.h>

using namespace muduo;
using namespace muduo::net;
using namespace std::placeholders;

std::map<string, string> redirections;

void onRequest(const HttpRequest& req, HttpResponse* resp) {
    LOG_INFO << "Headers "
             << req.methodString()
             << " "
             << req.path();
    // Al headers
    const std::map<string, string>& headers = req.headers();
    for(const auto& header : headers)
        LOG_INFO << header.first << " : " << header.second;

    // Find redirections
    auto it = redirections.find(req.path());
    if (it != redirections.end()) {
        resp->setStatusCode(HttpResponse::k301MovedPermanently);
        resp->setStatusMessage("Moved permanently");
        resp->addHeader("Location", it->second);
    } else if (req.path() == "/") {
        resp->setStatusCode(HttpResponse::k200Ok);
        resp->setStatusMessage("OK");
        resp->setContentType("text/html");
        string now = Timestamp::now().toFormattedString();
        string text;
        for (const auto& redi : redirections)
            text.append("<ul>" + redi.first + " =&gt; " + redi.second + "</ul>");
        resp->setBody("<html><head><title>My tiny short url service</title></head>"
            "<body><h1>Know redirections</h1>" + text + "Now is " + now + "</body></html>");
    } else {
        resp->setStatusCode(HttpResponse::k404NotFound);
        resp->setStatusMessage("Not Found");
        resp->setCloseConnection(true);
    }
}

int main(int argc, char* argv[]) {
    redirections["/1"] = "http://chuanjun.wang";
    redirections["/2"] = "http://zhihu.com";
    redirections["/3"] = "http://127.0.0.1:8000/4";
    redirections["/4"] = "http://127.0.0.1:8000/5";
    redirections["/5"] = "http://127.0.0.1:8000/3";

    int numThread = 4;
    EventLoop loop;
    EventLoopThreadPool threadPool(&loop, "shorturl");
    threadPool.setThreadNum(numThread);
    threadPool.start();

    std::vector<std::unique_ptr<HttpServer>> servers;
    for (int i = 0; i < numThread; ++i) {
        servers.emplace_back(new HttpServer(threadPool.getNextLoop(), InetAddress(8000),
            "shorturl", TcpServer::kReusePort));
        servers.back()->setHttpCallback(onRequest);
        servers.back()->getLoop()->runInLoop(std::bind(&HttpServer::start, servers.back().get()));
    }
    loop.loop();
}
