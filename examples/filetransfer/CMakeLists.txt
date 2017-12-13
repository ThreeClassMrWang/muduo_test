cmake_minimum_required(VERSION 3.5)

include_directories(../../muduo)

set(LIB_NEED muduo_base muduo_net pthread)

add_executable(download fileutil.cpp fileutil.h download.cpp)
target_link_libraries(download ${LIB_NEED})

add_executable(download2 fileutil.h fileutil.cpp download2.cpp)
target_link_libraries(download2 ${LIB_NEED})

add_executable(download3 fileutil.h fileutil.cpp download3.cpp)
target_link_libraries(download3 ${LIB_NEED})
