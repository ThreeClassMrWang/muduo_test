cmake_minimum_required(VERSION 3.5)

set(LIB_NEED muduo_base muduo_net protobuf pthread z)

include_directories(../../muduo)

add_executable(codec query.pb.h query.pb.cc Codec.cpp Codec.h Dispatcher.h)
target_link_libraries(codec ${LIB_NEED})