cmake_minimum_required(VERSION 3.0)
project(muduo_test)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O0 -g")

add_subdirectory(examples)
add_subdirectory(muduo)

set(EXECUTABLE_OUTPUT_PATH "${PROJECT_BINARY_DIR}/bin")
set(LIBRARY_OUTPUT_PATH  "${PROJECT_BINARY_DIR}/lib")

add_executable(muduo_test main.cpp)