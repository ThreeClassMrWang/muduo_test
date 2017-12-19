//
// Created by wcj on 12/18/17.
//

#ifndef MUDUO_TEST_TIMINGWHEEL_H
#define MUDUO_TEST_TIMINGWHEEL_H

#include <muduo/base/noncopyable.h>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <muduo/net/TcpConnection.h>
#include <muduo/net/EventLoop.h>
#include <muduo/base/Mutex.h>

/// Using RAII to manage TcpConnection
class Entry : muduo::copyable {
public:
    using WeakTcpConnectionPtr = std::weak_ptr<muduo::net::TcpConnection>;
    Entry (const WeakTcpConnectionPtr& weakConn) : weakConn_(weakConn) { }

    ~Entry() {
        muduo::net::TcpConnectionPtr conn = weakConn_.lock();
        if (conn)
            conn->shutdown();
    }

private:
    WeakTcpConnectionPtr weakConn_;
};

class CircularIndex : muduo::copyable {
public:
    CircularIndex(size_t index, size_t max) : index_(index), max_(max) { }
    ~CircularIndex() = default;

    void add(size_t value) {
        index_ =  (index_ + value) % max_;
    }

    void sub(size_t value) {
        index_ =  ((index_ - value) % max_ + max_) % max_;
    }

    void inc(void) { add(1); }
    void dec(void) { sub(1); }

    size_t get(void) { return index_; }
    size_t getInc(void) { return (index_ + 1) % max_; }
    size_t getDec(void) { return  ((index_ - 1) % max_ + max_) % max_; }

private:
    size_t index_;
    size_t max_;
};

class TimingWheel : muduo::noncopyable {
public:
    using EntryPtr = std::shared_ptr<Entry>;
    using WeakEntryPtr = std::weak_ptr<Entry>;

    TimingWheel(muduo::net::EventLoop* loop, size_t maxIdleTimeSecond);

    void add(const muduo::net::TcpConnectionPtr& conn);
    size_t update(const muduo::net::TcpConnectionPtr& conn, size_t lastUpdate);

private:
    using ConnectionSet = std::unordered_set<EntryPtr>;
    using WheelMap = std::unordered_map<size_t, ConnectionSet>;
    WheelMap wheelMap_;
    muduo::MutexLock mutex_;
    size_t maxIdleTimeSecond_;
    muduo::net::EventLoop* loop_;
    CircularIndex tail_;

    void onTimer();
};


#endif //MUDUO_TEST_TIMINGWHEEL_H
