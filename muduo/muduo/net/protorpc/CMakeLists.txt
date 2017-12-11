add_custom_command(OUTPUT rpc.pb.cc rpc.pb.h
  COMMAND protoc
  ARGS --cpp_out . ${CMAKE_CURRENT_SOURCE_DIR}/rpc.proto -I${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS rpc.proto
  VERBATIM )

set_source_files_properties(rpc.pb.cc PROPERTIES COMPILE_FLAGS "-Wno-conversion")
include_directories(${PROJECT_BINARY_DIR})

add_library(muduo_protorpc_wire rpc.pb.cc RpcCodec.cc)
set_target_properties(muduo_protorpc_wire PROPERTIES COMPILE_FLAGS "-Wno-error=shadow")

add_library(muduo_protorpc RpcChannel.cc RpcServer.cc)
set_target_properties(muduo_protorpc PROPERTIES COMPILE_FLAGS "-Wno-error=shadow")
target_link_libraries(muduo_protorpc muduo_protorpc_wire muduo_protobuf_codec muduo_net protobuf z)

if(TCMALLOC_LIBRARY)
  target_link_libraries(muduo_protorpc tcmalloc_and_profiler)
endif()
