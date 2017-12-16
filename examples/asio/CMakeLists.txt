cmake_minimum_required(VERSION 3.5)

include_directories(../../muduo)

set(LIB_NEED muduo_base muduo_net pthread)

add_executable(asio_server Codec.cpp Codec.h ChatServer.cpp ChatServer.h asio_server.cpp)
target_link_libraries(asio_server ${LIB_NEED})

add_executable(asio_client Codec.h Codec.cpp asio_client.cpp ChatClient.cpp ChatClient.h)
target_link_libraries(asio_client ${LIB_NEED})

add_executable(asio_server_hp Codec.h Codec.cpp asio_server_hp.cpp ChatServerHp.h ChatServerHp.cpp)
target_link_libraries(asio_server_hp ${LIB_NEED})