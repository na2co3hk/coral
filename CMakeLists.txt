cmake_minimum_required(VERSION 3.22.1)
project(coral)

set(CMAKE_CXX_STANDARD 20)

add_library(${PROJECT_NAME} SHARED
        src/base/annotation.hpp
        src/base/aop.hpp
        src/base/basic_buffer.hpp
        src/base/basic_data_structure.hpp
        src/base/json.h
        src/base/noncopyable.h
        src/base/string_handler.hpp
        src/coroutine/awaitable.hpp
        src/coroutine/awaiter.hpp
        src/coroutine/chan.hpp
        src/log/logger.hpp
        src/log/logstream.hpp
        src/network/http/cookie.hpp
        src/network/http/http_info.hpp
        src/network/http/http_request.hpp
        src/network/http/http_response.hpp
        src/network/http/http_router.hpp
        src/network/http/http_server.hpp
        src/network/inet_address.hpp
        src/network/io_context.cpp
        src/network/io_context.h
        src/network/loop_threadpool.hpp
        src/network/socket.cpp
        src/network/socket.h
        src/network/tcp_server.hpp
        src/test/unit_test.hpp include/coral.h src/network/tcp_client.hpp)

add_executable(Manjusaka src/main.cpp)
target_link_libraries(Manjusaka PUBLIC coral)

#添加coral.h
target_include_directories(coral PUBLIC include)

include_directories(include)