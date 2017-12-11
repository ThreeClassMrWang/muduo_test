cmake_minimum_required(VERSION 3.5)

set(LIB_NEEDED muduo_base muduo_net pthread)

add_executable(finger01 finger01.cpp)
target_link_libraries(finger01 ${LIB_NEEDED})

add_executable(finger02 finger02.cpp)
target_link_libraries(finger02 ${LIB_NEEDED})

add_executable(finger03 finger03.cpp)
target_link_libraries(finger03 ${LIB_NEEDED})

add_executable(finger04 finger04.cpp)
target_link_libraries(finger04 ${LIB_NEEDED})

add_executable(finger05 finger05.cpp)
target_link_libraries(finger05 ${LIB_NEEDED})