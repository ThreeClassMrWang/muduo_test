//
// Created by wcj on 12/17/17.
//

#ifndef MUDUO_TEST_DISPATCHER_H
#define MUDUO_TEST_DISPATCHER_H

#include <muduo/net/Callbacks.h>
#include <google/protobuf/message.h>
#include <muduo/base/noncopyable.h>
#include <map>
#include <type_traits>
#include "Codec.h"

class Callback : muduo::noncopyable {
public:
    virtual ~Callback() {};
    virtual void onMessage(const muduo::net::TcpConnectionPtr&,
            const Codec::MessagePtr&, muduo::Timestamp) = 0;
};

template <typename T>
class CallbackT : public Callback {
    static_assert(std::is_base_of<google::protobuf::Message, T>::value,
                  "T must be derived from gpb::Message");
public:
    using ProtobufMessageTCallback = std::function<void (muduo::net::TcpConnectionPtr&,
          const std::shared_ptr<T>&, muduo::Timestamp)>;

    CallbackT(const ProtobufMessageTCallback& callback) : callback_(callback) {}

    virtual void onMessage(const muduo::net::TcpConnectionPtr& conn,
                    const Codec::MessagePtr& message,
                    muduo::Timestamp receiveTime) const {
        std::shared_ptr<T> concrete = muduo::down_pointer_cast<T>(message);
        assert(concrete != nullptr);
        callback_(conn, concrete, receiveTime);
    }

private:
    ProtobufMessageTCallback callback_;
};

class Dispatcher {
public:
    using ProtobufMessageCallback = std::function<void(muduo::net::TcpConnectionPtr&,
        const Codec::MessagePtr&, muduo::Timestamp)>;

    Dispatcher(const ProtobufMessageCallback& defaultCb) :
            defaultCallback_(defaultCb) { }

    template <typename T>
    void registerMessageCallback(const typename CallbackT<T>::ProtobufMessageTCallback& callback) {
        std::shared_ptr<CallbackT<T>> pd = std::make_shared(callback);
        callbacks_[T::descriptor()] = pd;
    }

    void onProtobufMessage(const muduo::net::TcpConnectionPtr& conn,
            const Codec::MessagePtr& msg, muduo::Timestamp receiveTime) {
        auto it = callbacks_.find(msg->GetDescriptor());
        if (it != callbacks_.end()) {
            it->second->onMessage(conn, msg, receiveTime);
        } else {
            defaultCallback_(conn, msg, receiveTime);
        }
    }

private:
    using CallbackMap = std::map<const google::protobuf::Descriptor*,
            std::shared_ptr<Callback>>;
    CallbackMap callbacks_;
    ProtobufMessageCallback defaultCallback_;
};

#endif //MUDUO_TEST_DISPATCHER_H
