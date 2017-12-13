cmake_minimum_required(VERSION 3.5)

include_directories(../../muduo)

set(LIB_NEED muduo_base muduo_net pthread)

add_executable(simple_echo simple_echo.cpp)
target_link_libraries(simple_echo ${LIB_NEED})

add_executable(simple_discard simple_discard.cpp)
target_link_libraries(simple_discard ${LIB_NEED})

add_executable(simple_daytime simple_daytime.cpp)
target_link_libraries(simple_daytime ${LIB_NEED})

add_executable(simple_time simple_time.cpp)
target_link_libraries(simple_time ${LIB_NEED})

add_executable(simple_timeclient simple_timeclient.cpp)
target_link_libraries(simple_timeclient ${LIB_NEED})

add_executable(simple_chargen simple_chargen.cpp)
target_link_libraries(simple_chargen ${LIB_NEED})