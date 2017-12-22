//
// Created by wcj on 12/22/17.
//

#include "Multiplexer.h"

using namespace muduo::net;
using namespace muduo;

static constexpr uint16_t defaultServerPort = 3333;
static constexpr char defaultBackendIp[] = {"127.0.0.1"};
static constexpr uint16_t defaultBackendPort = 9999;
static constexpr int defaultNumThread = 4;

int main(int argc, char* argv[]) {
    EventLoop loop;
    InetAddress listenAddr(defaultServerPort);
    InetAddress backendAddr(defaultBackendIp, defaultBackendPort);
    Multiplexer multiplexer(&loop, listenAddr, backendAddr, defaultNumThread);
    multiplexer.start();
    loop.loop();
}