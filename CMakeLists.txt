cmake_minimum_required(VERSION 3.16)
project(sylar-nanasaki)

include(cmake/utils.cmake)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_C_COMPILE /usr/bin/clang)
set(CMAKE_CXX_COMPILE /usr/bin/clang++)

# -O0：关闭所有优化
# -ggdb：启用调试信息的生成
# -Werror：将所有警告视为错误
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -std=c++2a -O0 -ggdb -Wall -Werror")

# -fPIC: 生成位置无关的代码，便于动态链接
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

# -Wno-unused-function: 不要警告未使用函数
# -Wno-builtin-macro-redefined: 不要警告内置宏重定义，用于重定义内置的__FILE__宏
# -Wno-deprecated: 不要警告过时的特性
# -Wno-deprecated-declarations: 不要警告使用带deprecated属性的变量，类型，函数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated -Wno-deprecated-declarations")

# -rdynamic: 将所有符号都加入到符号表中，便于使用dlopen或者backtrace追踪到符号
set(RDYNAMIC_FLAG "-rdynamic")

include_directories(.)

set(LIB_SRC
    sylar/log.cc
    sylar/mutex.cc
    sylar/env.cc
    sylar/config.cc
    sylar/util.cc
    sylar/thread.cc
    sylar/fiber.cc
    sylar/scheduler.cc
    sylar/timer.cc
    sylar/iomanager.cc
    sylar/hook.cc
    sylar/fd_manager.cc
    sylar/address.cc
    sylar/socket.cc
    sylar/bytearray.cc
)

find_package(Boost REQUIRED)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

add_library(sylar SHARED ${LIB_SRC})
target_link_options(sylar PRIVATE ${RDYNAMIC_FLAG})
force_redefine_file_macro_for_sources(sylar)

set(LIBS
    sylar
    pthread
    dl
    yaml-cpp
)

# 测试
option(BUILD_TEST "ON for complile test" ON)
if(BUILD_TEST)
    sylar_add_executable(test_log "tests/test_log.cpp" sylar "${LIBS}")
    sylar_add_executable(test_env "tests/test_env.cpp" sylar "${LIBS}")
    sylar_add_executable(test_config "tests/test_config.cpp" sylar "${LIBS}")
    sylar_add_executable(test_thread "tests/test_thread.cpp" sylar "${LIBS}")
    sylar_add_executable(test_fiber "tests/test_fiber.cpp" sylar "${LIBS}")
    sylar_add_executable(test_scheduler "tests/test_scheduler.cpp" sylar "${LIBS}")
    sylar_add_executable(test_iomanager "tests/test_iomanager.cpp" sylar "${LIBS}")
    sylar_add_executable(test_timer "tests/test_timer.cpp" sylar "${LIBS}")
    sylar_add_executable(test_hook "tests/test_hook.cpp" sylar "${LIBS}")
    sylar_add_executable(test_address "tests/test_address.cpp" sylar "${LIBS}")
    sylar_add_executable(test_socket_tcp_client "tests/test_socket_tcp_client.cpp" sylar "${LIBS}")
    sylar_add_executable(test_socket_tcp_server "tests/test_socket_tcp_server.cpp" sylar "${LIBS}")
    sylar_add_executable(test_bytearray "tests/test_bytearray.cpp" sylar "${LIBS}") 
endif()

# 输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
