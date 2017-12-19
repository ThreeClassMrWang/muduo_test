cmake_minimum_required(VERSION 3.0)

include_directories(../../muduo)

set(LIB_NEED muduo_base muduo_net pthread)

add_executable(timingwheel TimingWheel.cpp TimingWheel.h DiscardServer.cpp)
target_link_libraries(timingwheel ${LIB_NEED})