/*
 * @Author: Nana5aki
 * @Date: 2024-11-30 16:28:05
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-01-01 22:15:47
 * @FilePath: /MySylar/sylar/util.cc
 */

#include "util.h"
#include "log.h"
#include <cstring>
#include <dirent.h>
#include <execinfo.h>   // for backtrace()
#include <pthread.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

namespace sylar {

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

pid_t GetThreadId() {
    return syscall(SYS_gettid);
}

uint64_t GetFiberId() {
    return 0;
}

uint64_t GetElapsedMS() {
    struct timespec ts = {0};
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

std::string GetThreadName() {
    char thread_name[16] = {0};
    pthread_getname_np(pthread_self(), thread_name, 16);
    return std::string(thread_name);
}

void SetThreadName(const std::string& name) {
    pthread_setname_np(pthread_self(), name.substr(0, 15).c_str());
}

static std::string demangle(const char* str) {
    size_t size = 0;
    int status = 0;
    std::string rt;
    rt.resize(256);
    if (1 == sscanf(str, "%*[^(]%*[^_]%255[^)+]", &rt[0])) {
        char* v = abi::__cxa_demangle(&rt[0], nullptr, &size, &status);
        if (v) {
            std::string result(v);
            free(v);
            return result;
        }
    }
    if (1 == sscanf(str, "%255s", &rt[0])) {
        return rt;
    }
    return str;
}

void Backtrace(std::vector<std::string>& bt, int size, int skip) {
    void** array = (void**)malloc((sizeof(void*) * size));
    // backtrace()函数，获取函数调用堆栈帧数据，即回溯函数调用列表。
    // 数据将放在buffer中。参数size用来指定buffer中可以保存多少个void*元素（表示相应栈帧的地址，一个返回地址）
    // 如果回溯的函数调用大于size，则size个函数调用地址被返回。为了取得全部的函数调用列表，应保证buffer和size足够大
    size_t s = ::backtrace(array, size);
    // 将从backtrace()函数获取的地址转为描述这些地址的字符串数组
    // 每个地址的字符串信息包含对应函数的名字、在函数内的十六进制偏移地址、以及实际的返回地址（十六进制）
    char** strings = backtrace_symbols(array, s);
    if (strings == NULL) {
        SYLAR_LOG_ERROR(g_logger) << "backtrace_synbols error";
        return;
    }

    for (size_t i = skip; i < s; ++i) {
        bt.push_back(demangle(strings[i]));
    }

    free(strings);
    free(array);
}

std::string BacktraceToString(int size, int skip, const std::string& prefix) {
    std::vector<std::string> bt;
    Backtrace(bt, size, skip);
    std::stringstream ss;
    for (size_t i = 0; i < bt.size(); ++i) {
        ss << prefix << bt[i] << std::endl;
    }
    return ss.str();
}

void FSUtil::ListAllFile(std::vector<std::string>& files, const std::string& path,
                         const std::string& subfix) {
    if (access(path.c_str(), 0) != 0) {
        return;
    }
    DIR* dir = opendir(path.c_str());
    if (dir == nullptr) {
        return;
    }
    struct dirent* dp = nullptr;
    while ((dp = readdir(dir)) != nullptr) {
        if (dp->d_type == DT_DIR) {
            if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..")) {
                continue;
            }
            ListAllFile(files, path + "/" + dp->d_name, subfix);
        } else if (dp->d_type == DT_REG) {
            std::string filename(dp->d_name);
            if (subfix.empty()) {
                files.push_back(path + "/" + filename);
            } else {
                if (filename.size() < subfix.size()) {
                    continue;
                }
                if (filename.substr(filename.length() - subfix.size()) == subfix) {
                    files.push_back(path + "/" + filename);
                }
            }
        }
    }
    closedir(dir);
}

}   // namespace sylar
