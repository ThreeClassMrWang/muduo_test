cmake_minimum_required(VERSION 3.5)

set(LIB_NEED muduo_base muduo_net protobuf pthread z)

include_directories(../../muduo)

add_executable(protobufServer protobufServer.cpp query.pb.h query.pb.cc Codec.cpp Codec.h Dispatcher.h)
target_link_libraries(protobufServer ${LIB_NEED})

add_executable(protobufClient protobufClient.cpp query.pb.h query.pb.cc Codec.cpp Codec.h Dispatcher.h)
target_link_libraries(protobufClient ${LIB_NEED})