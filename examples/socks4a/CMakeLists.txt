cmake_minimum_required(VERSION 3.0)

include_directories(../../muduo)

set(LIB_NEED muduo_base muduo_net pthread)

add_executable(sock4a sock4a.cpp Sock4aServer.cpp Sock4aServer.h)
target_link_libraries(sock4a ${LIB_NEED})

add_executable(sock4a_test sock4a_test.cpp)
target_link_libraries(sock4a_test ${LIB_NEED})
