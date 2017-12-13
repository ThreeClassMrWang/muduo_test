cmake_minimum_required(VERSION 3.5)

include_directories(../../muduo)

add_executable(sudoku sudoku.cpp sudoku.h server_basic.cpp)
target_link_libraries(sudoku muduo_base muduo_net pthread)

add_executable(sudoku_threadpool sudoku.h sudoku.cpp server_threadpool.cpp)
target_link_libraries(sudoku_threadpool muduo_base muduo_net pthread)