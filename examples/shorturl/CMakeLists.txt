cmake_minimum_required(VERSION 3.0)

include_directories(../../muduo)

set(LIB_NEED muduo_base muduo_net muduo_http pthread)

add_executable(shorturl shorturl.cpp)
target_link_libraries(shorturl ${LIB_NEED})