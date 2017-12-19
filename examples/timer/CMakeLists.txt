cmake_minimum_required(VERSION 3.0)

include_directories(../../muduo)

set(LIB_NEED muduo_base muduo_net pthread)

add_executable(timer2 timer2.cpp)
target_link_libraries(timer2 ${LIB_NEED})

add_executable(timer3 timer3.cpp)
target_link_libraries(timer3 ${LIB_NEED})

add_executable(timer4 timer4.cpp)
target_link_libraries(timer4 ${LIB_NEED})

add_executable(timer5 timer5.cpp)
target_link_libraries(timer5 ${LIB_NEED})
