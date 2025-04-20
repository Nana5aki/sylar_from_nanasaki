/*
 * @Author: Nana5aki
 * @Date: 2024-11-30 16:27:59
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-04-20 11:56:51
 * @FilePath: /MySylar/sylar/util.h
 */
#ifndef __SYLAR_UTIL_H__
#define __SYLAR_UTIL_H__

#include <cxxabi.h>
#include <stdint.h>
#include <string>
#include <sys/types.h>
#include <time.h>
#include <vector>

namespace sylar {

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
 * @brief 日期时间转字符串
 */
std::string Time2Str(time_t ts = time(0), const std::string& format = "%Y-%m-%d %H:%M:%S");

/**
 * @brief 字符串转日期时间
 */
time_t Str2Time(const char* str, const char* format = "%Y-%m-%d %H:%M:%S");

class FSUtil {
public:
  /**
   * @brief
   * 递归列举指定目录下所有指定后缀的常规文件，如果不指定后缀，则遍历所有文件，返回的文件名带路径
   * @param[out] files 文件列表
   * @param[in] path 路径
   * @param[in] subfix 后缀名，比如 ".yml"
   */
  static void ListAllFile(std::vector<std::string>& files, const std::string& path,
                          const std::string& subfix);

  /**
   * @brief 删除文件，参考unlink(2)
   * @param[in] filename 文件名
   * @param[in] exist 是否存在
   * @return  是否成功
   * @note 内部会判断一次是否真的不存在该文件
   */
  static bool Unlink(const std::string& filename, bool exist = false);
};

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

class StringUtil {
public:
  /**
   * @brief url编码
   * @param[in] str 原始字符串
   * @param[in] space_as_plus 是否将空格编码成+号，如果为false，则空格编码成%20
   * @return 编码后的字符串
   */
  static std::string UrlEncode(const std::string& str, bool space_as_plus = true);

  /**
   * @brief url解码
   * @param[in] str url字符串
   * @param[in] space_as_plus 是否将+号解码为空格
   * @return 解析后的字符串
   */
  static std::string UrlDecode(const std::string& str, bool space_as_plus = true);

  /**
   * @brief 移除字符串首尾的指定字符串
   * @param[] str 输入字符串
   * @param[] delimit 待移除的字符串
   * @return  移除后的字符串
   */
  static std::string Trim(const std::string& str, const std::string& delimit = " \t\r\n");

  /**
   * @brief 移除字符串首部的指定字符串
   * @param[] str 输入字符串
   * @param[] delimit 待移除的字符串
   * @return  移除后的字符串
   */
  static std::string TrimLeft(const std::string& str, const std::string& delimit = " \t\r\n");

  /**
   * @brief 移除字符尾部的指定字符串
   * @param[] str 输入字符串
   * @param[] delimit 待移除的字符串
   * @return  移除后的字符串
   */
  static std::string TrimRight(const std::string& str, const std::string& delimit = " \t\r\n");
};

}   // namespace sylar


#endif