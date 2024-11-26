# 重定义目标源码的__FILE__宏，使用相对路径的形式，避免暴露敏感信息

# This function will overwrite the standard predefined macro "__FILE__".
# "__FILE__" expands to the name of the current input file, but cmake
# input the absolute path of source file, any code using the macro 
# would expose sensitive information, such as MORDOR_THROW_EXCEPTION(x),
# so we'd better overwirte it with filename.
function(force_redefine_file_macro_for_sources targetname)
    # 获取指定目标的所有源文件，并存储在变量 source_files 中
    get_target_property(source_files "${targetname}" SOURCES)
    # 遍历 source_files 中的每个源文件
    foreach(sourcefile ${source_files})
        # 获取当前源文件的编译定义列表（COMPILE_DEFINITIONS 属性），存储在变量 defs 中
        get_property(defs SOURCE "${sourcefile}"
            PROPERTY COMPILE_DEFINITIONS)
        # 将当前 sourcefile 转换为绝对路径，存储在变量 filepath 中
        get_filename_component(filepath "${sourcefile}" ABSOLUTE)
        # 将 filepath 转换为相对于项目根目录（PROJECT_SOURCE_DIR）的路径，并存储在 relpath 
        string(REPLACE ${PROJECT_SOURCE_DIR}/ "" relpath ${filepath})
        # 在 defs 中追加一个定义：将 __FILE__ 宏重新定义为相对路径
        list(APPEND defs "__FILE__=\"${relpath}\"")
        # 将更新后的 defs 设置为当前源文件的 COMPILE_DEFINITIONS 属性。这样，每个源文件都会有一个特定的 __FILE__ 定义。
        set_property(
            SOURCE "${sourcefile}"
            PROPERTY COMPILE_DEFINITIONS ${defs}
            )
    endforeach()
endfunction()

# 封装 add_executable
# targetname: 可执行目标的名称。
# srcs: 源文件列表。
# depends: 目标的依赖项。
# libs: 目标需要链接的库
function(sylar_add_executable targetname srcs depends libs)
    add_executable(${targetname} ${srcs})
    add_dependencies(${targetname} ${depends})
    force_redefine_file_macro_for_sources(${targetname})
    target_link_libraries(${targetname} ${libs})
endfunction()
