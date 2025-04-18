# 配置http-parser
set(HTTP_PARSER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/http-parser)
set(HTTP_PARSER_SRC
    ${HTTP_PARSER_DIR}/http_parser.c
)

# 创建http-parser库
add_library(http_parser STATIC ${HTTP_PARSER_SRC})

# 设置http-parser的编译选项
set_target_properties(http_parser PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    C_STANDARD 99
)

# 导出http-parser的头文件目录
target_include_directories(http_parser PUBLIC ${HTTP_PARSER_DIR}) 