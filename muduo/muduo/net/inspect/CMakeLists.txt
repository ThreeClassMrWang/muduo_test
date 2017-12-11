set(inspect_SRCS
  Inspector.cc
  PerformanceInspector.cc
  ProcessInspector.cc
  SystemInspector.cc
  )

add_library(muduo_inspect ${inspect_SRCS})
target_link_libraries(muduo_inspect muduo_http)

if(TCMALLOC_INCLUDE_DIR AND TCMALLOC_LIBRARY)
  set_target_properties(muduo_inspect PROPERTIES COMPILE_FLAGS "-DHAVE_TCMALLOC")
  target_link_libraries(muduo_inspect tcmalloc_and_profiler)
endif()

