set(http_SRCS
  HttpServer.cc
  HttpResponse.cc
  HttpContext.cc
  )

add_library(muduo_http ${http_SRCS})
target_link_libraries(muduo_http muduo_net)
