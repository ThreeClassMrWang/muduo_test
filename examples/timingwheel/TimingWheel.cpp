//
// Created by wcj on 12/18/17.
//

#include <muduo/base/Logging.h>
#include "TimingWheel.h"

TimingWheel::TimingWheel(muduo::net::EventLoop* loop, size_t maxIdleTimeSecond) :
            maxIdleTimeSecond_(maxIdleTimeSecond), loop_(loop),
            tail_(maxIdleTimeSecond_ - 1, maxIdleTimeSecond_) {
    loop_->runEvery(1, std::bind(&TimingWheel::onTimer, this));
}
    
void TimingWheel::add(const muduo::net::TcpConnectionPtr& conn) {
    if (!conn->connected()) return;

    EntryPtr entryPtr(new Entry(conn));
    WeakEntryPtr weakEntryPtr(entryPtr);
    conn->setContext(weakEntryPtr);

    LOG_INFO << "add " << tail_.get();
    muduo::MutexLockGuard lock(mutex_);
    // Note: cannot use like that
    // auto connectionSet = wheelMap_[tail_.get()];     // This is copy
    // connectionSet.insert(entryPtr);
    // This is error usage?? why
    // Date: 2017.12.20, I know why, the code below is correct
    // auto &connectionSet = wheelMap_[tail_.get()];    // This is reference
    // connectionSet.insert(entryPtr);
    wheelMap_[tail_.get()].insert(entryPtr);
    LOG_INFO << "entry use count: " << entryPtr.use_count();
}

size_t TimingWheel::update(const muduo::net::TcpConnectionPtr &conn, size_t lastUpdate) {
    if (!conn->connected()) return 0;
    size_t res = 0;
    {
        muduo::MutexLockGuard lock(mutex_);
        res = tail_.get();
        if (lastUpdate == res)
            return res;
    }
    WeakEntryPtr weakEntryPtr = boost::any_cast<WeakEntryPtr>(conn->getContext());
    EntryPtr entryPtr = weakEntryPtr.lock();
    if (entryPtr) {
        LOG_INFO << "add " << tail_.get();
        muduo::MutexLockGuard lock(mutex_);
        wheelMap_[tail_.get()].insert(entryPtr);
        LOG_INFO << "entry use count: " << entryPtr.use_count();
    }

    return res;
}

void TimingWheel::onTimer() {
    LOG_INFO << "onTimer, erase " << tail_.getDec();
    muduo::MutexLockGuard lock(mutex_);
    wheelMap_.erase(tail_.getDec());
    tail_.dec();
}
