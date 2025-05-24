cmake_minimum_required(VERSION 3.5)

set(CMAKE_C_COMPILER /usr/bin/clang)
set(CMAKE_CXX_COMPILER /usr/bin/clang++)

project(sylar-nanasaki)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/utils.cmake)

# set(CMAKE_VERBOSE_MAKEFILE ON)

add_subdirectory(third_party/yaml-cpp)
add_subdirectory(third_party/tinyxml2)
add_subdirectory(third_party/jsoncpp)

# -O0：关闭所有优化
# -ggdb：启用调试信息的生成
# -Werror：将所有警告视为错误
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -std=c++17 -O0 -ggdb -Wall -Werror")

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
include_directories(${CMAKE_SOURCE_DIR}/third_party/yaml-cpp/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/tinyxml2)
include_directories(${CMAKE_SOURCE_DIR}/third_party/jsoncpp/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/http-parser)

# 自动搜索所有源文件和头文件
file(GLOB LIB_SRC
    "sylar/*.cc"
    "sylar/db/*.cc"
    "sylar/http/*.cc"
    "sylar/util/*.cc"
    "sylar/streams/*.cc"
    "third_party/http-parser/*.c"
)

find_package(Boost REQUIRED)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

find_package(SQLite3 REQUIRED)
if(SQLite3_FOUND)
    include_directories(${SQLite3_INCLUDE_DIRS})
endif()

add_library(sylar SHARED ${LIB_SRC})
target_link_options(sylar PRIVATE ${RDYNAMIC_FLAG})
force_redefine_file_macro_for_sources(sylar)

set(LIBS
    sylar
    pthread
    dl
    yaml-cpp
    tinyxml2
    jsoncpp
    ${SQLite3_LIBRARIES}
)

# 输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# ORM
file(GLOB ORM_SRC "sylar/orm/*.cc" "sylar/orm/orm.cpp")
sylar_add_executable(gen_orm "${ORM_SRC}" sylar "${LIBS}")

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/bin/orm_out")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/bin/orm_out)
    set(LIBS ${LIBS} orm_data)
    set(ORM_EXIST 1)
    message(STATUS "ORM_EXIST: ${ORM_EXIST}")
endif()

# 测试
option(BUILD_TEST "ON for complile test" ON)
if(BUILD_TEST)
    set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/test_out)
    # 自动查找所有 tests 目录下以 test_ 开头的 cpp 文件
    file(GLOB TEST_SOURCES "tests/test_*.cpp")
    foreach(test_src ${TEST_SOURCES})
        # 获取文件名（不带路径和扩展名）
        get_filename_component(test_name ${test_src} NAME_WE)
        if("${test_name}" STREQUAL "test_orm")
            if("${ORM_EXIST}" STREQUAL "1")
                sylar_add_executable(${test_name} ${test_src} orm_data "${LIBS}")
            endif()
            continue()
        endif()
        sylar_add_executable(${test_name} ${test_src} sylar "${LIBS}")
    endforeach()
endif()
