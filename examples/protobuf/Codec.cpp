//
// Created by wcj on 12/17/17.
//

#include <muduo/base/Logging.h>
#include <zlib.h>
#include <muduo/net/protorpc/google-inl.h>
#include "Codec.h"

using namespace muduo::net;
using namespace muduo;

namespace detail {
    static const string kNoErrorStr = "NoError";
    static const string kInvalidLengthStr = "InvalidLength";
    static const string kCheckSumErrorStr = "CheckSumError";
    static const string kInvalidNameLenStr = "InvalidNameLen";
    static const string kUnknowMessageTypeStr = "UnknowMessageTypeError";
    static const string kParseErrorStr = "ParseError";
    static const string kUnknowError = "UnknowError";
}

const string& Codec::errorCodeToString(ErrorCode errorCode) {
    switch (errorCode) {
        case kNoError:
            return detail::kNoErrorStr;
        case kInvalidLength:
            return detail::kInvalidLengthStr;
        case kCheckSumError:
            return detail::kCheckSumErrorStr;
        case kInvalidNameLen:
            return detail::kInvalidNameLenStr;
        case kUnknowMessageType:
            return detail::kUnknowMessageTypeStr;
        case kParseError:
            return detail::kParseErrorStr;
        default:
            return detail::kUnknowError;
    }
}

void Codec::defaultErrorCallback(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buff,
                                 muduo::Timestamp receiveTime, ErrorCode errorCode) {
    LOG_ERROR << conn->name()
              << __PRETTY_FUNCTION__
              << " - "
              << errorCodeToString(errorCode);
    if (conn && conn->connected())
        conn->shutdown();
}

void Codec::onMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buff,
                      muduo::Timestamp receiveTime) {
    while (buff->readableBytes() >= kMinMessageLen + kHeaderLen) {
        const int32_t len = buff->peekInt32();
        if (len > kMaxMessageLen || len < kMinMessageLen) {
            errorCallback_(conn, buff, receiveTime, kInvalidLength);
        } else if (buff->readableBytes() >= static_cast<size_t>(len + kHeaderLen)) {
            ErrorCode errorCode = kNoError;
            MessagePtr message = parse(buff->peek()+kHeaderLen, len, &errorCode);
            if (errorCode == kNoError && message) {
                messageCallback_(conn, message, receiveTime);
                buff->retrieve(static_cast<size_t>(kHeaderLen + len));
            } else {
                errorCallback_(conn, buff, receiveTime, errorCode);
                break;
            }
        } else {
            break;
        }
    }
}

google::protobuf::Message* Codec::createMessage(const std::string &typeName) {
    google::protobuf::Message* msg = nullptr;
    const google::protobuf::Descriptor* descriptor =
            google::protobuf::DescriptorPool::generated_pool()->
            FindMessageTypeByName(typeName);
    if (descriptor) {
        const google::protobuf::Message* prototype =
                google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
        if (prototype)  msg = prototype->New();
    }

    return msg;
}


Codec::MessagePtr Codec::parse(const char *buf, int len, ErrorCode *errorCode) {
    MessagePtr message;

    int32_t be32 = *(static_cast<int32_t *>(buf + len - kHeaderLen));
    int32_t expectCheckSum = muduo::net::sockets::networkToHost32(static_cast<uint32_t>(be32));
    int32_t checkSum = static_cast<int32_t> (::adler32(1, reinterpret_cast<const Bytef*>(buf),
                                                       static_cast<uInt>(len - kHeaderLen)));
    if (checkSum == expectCheckSum) {
        be32 = *(static_cast<int32_t *>(buf));
        int32_t nameLen = muduo::net::sockets::networkToHost32(static_cast<uint32_t>(be32));
        if (nameLen >= 2 && nameLen <= len - 2*kHeaderLen) {
            std::string typeName(buf + kHeaderLen, buf + kHeaderLen + nameLen - 1);
            // Create message object
            message.reset(createMessage(typeName));
            if (message) {
                // Parse from buffer
                const char* data = buf + kHeaderLen + nameLen;
                int32_t dataLen = len - nameLen - 2*kHeaderLen;
                if (message->ParseFromArray(data, dataLen))
                    *errorCode = kNoError;
                else
                    *errorCode = kParseError;
            } else {
                *errorCode = kUnknowMessageType;
            }
        } else {
            *errorCode = kInvalidLength;
        }
    } else {
        *errorCode = kCheckSumError;
    }

    return message;
}

void Codec::fillEmptyBuffer(muduo::net::Buffer *buff, const google::protobuf::Message &message) {
    const std::string& typeName = message.GetTypeName();
    int32_t nameLen = static_cast<int32_t>(typeName.size() + 1);
    buff->appendInt32(nameLen);
    buff->append(typeName.c_str(), static_cast<size_t>(nameLen));

    GOOGLE_DCHECK(message.IsInitialized()) << InitializationErrorMessage("serialize", message);

    int byteSize = message.ByteSize();
    buff->ensureWritableBytes(static_cast<size_t>(byteSize));

    uint8_t* start = reinterpret_cast<uint8_t *>(buff->beginWrite());
    uint8_t* end = message.SerializeWithCachedSizesToArray(start);
    if (end - start != byteSize)
        ByteSizeConsistencyError(byteSize, message.ByteSize(), static_cast<int>(end - start));
    buff->hasWritten(static_cast<size_t>(byteSize));

    int32_t checkSum = static_cast<int32_t>(::adler32(1, reinterpret_cast<const Bytef*>(buff->peek()),
            static_cast<int>(buff->readableBytes())));
    buff->appendInt32(checkSum);
    int32_t len = sockets::hostToNetwork32(static_cast<int32_t>(buff->readableBytes()));
    buff->prepend(&len, sizeof(len));
}