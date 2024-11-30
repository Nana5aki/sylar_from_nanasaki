/*
 * @Author: Nana5aki
 * @Date: 2024-11-30 16:28:05
 * @LastEditors: Nana5aki
 * @LastEditTime: 2024-11-30 16:46:52
 * @FilePath: /MySylar/sylar/util.cc
 */

#include "util.h"
#include <pthread.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

namespace sylar {

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

}   // namespace sylar
