//
// Created by wcj on 12/8/17.
//

#include <muduo/net/EventLoop.h>

/// Do nothing, just wait.
///
/// \param argc
/// \param argv
/// \return
int main(int argc, char *argv[]) {
    muduo::net::EventLoop loop;
    loop.loop();
}
