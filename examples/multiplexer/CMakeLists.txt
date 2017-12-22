cmake_minimum_required(VERSION 3.0)

include_directories(../../muduo)

set(LIB_NEED muduo_base muduo_net pthread)

add_executable(multiplexer Multiplexer.h Multiplexer.cpp multiplexer.cpp)
target_link_libraries(multiplexer ${LIB_NEED})

add_executable(backend backend.cpp)
target_link_libraries(backend ${LIB_NEED})