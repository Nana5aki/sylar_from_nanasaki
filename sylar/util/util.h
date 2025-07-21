/*
 * @Author: Nana5aki
 * @Date: 2025-05-18 15:36:33
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-05-18 17:58:22
 * @FilePath: /sylar_from_nanasaki/sylar/util/util.h
 */
#pragma once

#include "lexical_cast.h"
#include <cxxabi.h>
#include <stdint.h>
#include <string>
#include <sys/types.h>
#include <time.h>
#include <vector>

namespace sylar {
namespace util {

/**
 * @brief 获取线程id
 * @note 这里不要把pid_t和pthread_t混淆，关于它们之的区别可参考gettid(2)
 */
pid_t GetThreadId();

/**
 * @brief 获取协程id
 * @todo 桩函数，暂时返回0，等协程模块完善后再返回实际值
 */
uint64_t GetFiberId();

/**
 * @brief 获取当前启动的毫秒数，参考clock_gettime(2)，使用CLOCK_MONOTONIC_RAW
 */
uint64_t GetElapsedMS();

/**
 * @brief 获取线程名称，参考pthread_getname_np(3)
 */
std::string GetThreadName();

/**
 * @brief 设置线程名称，参考pthread_setname_np(3)
 * @note 线程名称不能超过16字节，包括结尾的'\0'字符
 */
void SetThreadName(const std::string& name);

/**
 * @brief 获取当前的调用栈
 * @param[out] bt 保存调用栈
 * @param[in] size 最多返回层数
 * @param[in] skip 跳过栈顶的层数
 */
void Backtrace(std::vector<std::string>& bt, int size = 64, int skip = 1);

/**
 * @brief 获取当前栈信息的字符串
 * @param[in] size 栈的最大层数
 * @param[in] skip 跳过栈顶的层数
 * @param[in] prefix 栈信息前输出的内容
 */
std::string BacktraceToString(int size = 64, int skip = 2, const std::string& prefix = "");

/**
 * @brief 获取当前时间的毫秒
 */
uint64_t GetCurrentMS();

/**
 * @brief 获取当前时间的微秒
 */
uint64_t GetCurrentUS();

/**
 * @brief 日期时间转字符串
 */
std::string Time2Str(time_t ts = time(0), const std::string& format = "%Y-%m-%d %H:%M:%S");

/**
 * @brief 字符串转日期时间
 */
time_t Str2Time(const char* str, const char* format = "%Y-%m-%d %H:%M:%S");

/**
 * @brief 获取T类型的类型字符串
 */
template <class T>
const char* TypeToName() {
  // abi::__cxa_demangle 是 C++ ABI (Application Binary Interface)
  // 中的一个函数，用于将编译器产生的 C++ 符号名还原为人类可读的形式。在 C++
  // 中，函数名、类名等在编译过程中会被编译器改写为一种编码形式，这种形式不易阅读，而
  // abi::__cxa_demangle可以将其还原为正常的形式，便于理解和调试。具体来说，abi::__cxa_demangle
  // 函数的作用是将 C++ 符号名（mangled name）转换为其原始形式
  static const char* s_name = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
  return s_name;
}

template <class V, class Map, class K>
V GetParamValue(const Map& m, const K& k, const V& def = V()) {
  auto it = m.find(k);
  if (it == m.end()) {
    return def;
  }
  try {
    return sylar::util::lexical_cast<V>(it->second);
  } catch (...) {}
  return def;
}

template <class V, class Map, class K>
bool CheckGetParamValue(const Map& m, const K& k, V& v) {
  auto it = m.find(k);
  if (it == m.end()) {
    return false;
  }
  try {
    v = sylar::util::lexical_cast<V>(it->second);
    return true;
  } catch (...) {}
  return false;
}

}   // namespace util
}   // namespace sylar