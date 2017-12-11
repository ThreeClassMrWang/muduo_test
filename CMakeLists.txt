cmake_minimum_required(VERSION 3.9)
project(muduo_test)

set(CMAKE_CXX_STANDARD 11)

add_subdirectory(examples)
add_subdirectory(muduo)

set(EXECUTABLE_OUTPUT_PATH "${PROJECT_BINARY_DIR}/bin")
set(LIBRARY_OUTPUT_PATH  "${PROJECT_BINARY_DIR}/lib")

add_executable(muduo_test main.cpp)