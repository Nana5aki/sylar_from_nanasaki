/*
 * @Author: Nana5aki
 * @Date: 2024-11-30 15:44:46
 * @LastEditors: Nana5aki
 * @LastEditTime: 2024-11-30 16:42:13
 * @FilePath: /MySylar/sylar/env.cc
 */
#include "env.h"
#include "log.h"
#include <cstdlib>
#include <unistd.h>

namespace sylar {

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

bool Env::init(int argc, char** argv) {
    char link[1024] = {0};
    char path[1024] = {0};
    // /proc/pid/exe 一个符号连接->指向进程ID为pid的进程的可执行文件
    sprintf(link, "/proc/%d/exe", getpid());
    // readlink读取符号连接的内容并将其存储在缓冲区
    readlink(link, path, sizeof(path));
    // 上面两行获得程序bin文件绝对路径
    //  /path/xxx/exe
    m_exe = path;

    // 获得当前程序工作目录
    auto pos = m_exe.find_last_of("/");
    m_cwd = m_exe.substr(0, pos) + "/";

    // 程序名字，即文件名
    m_program = argv[0];

    // 分析传入参数（程序环境变量）
    // -config /path/to/config -file xxxx -d
    const char* now_key = nullptr;
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            if (strlen(argv[i]) > 1) {
                if (now_key) {
                    add(now_key, "");
                }
                now_key = argv[i] + 1;
            } else {
                SYLAR_LOG_ERROR(g_logger) << "invalid arg idx=" << i << " val=" << argv[i];
                return false;
            }
        } else {
            if (now_key) {
                add(now_key, argv[i]);
                now_key = nullptr;
            } else {
                SYLAR_LOG_ERROR(g_logger) << "invalid arg idx=" << i << " val=" << argv[i];
                return false;
            }
        }
    }
    if (now_key) {
        add(now_key, "");
    }
    return true;
}

}   // namespace sylar
