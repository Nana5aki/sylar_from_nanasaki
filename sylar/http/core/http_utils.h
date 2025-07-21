/*
 * @Author: Nana5aki
 * @Date: 2025-04-18 23:38:04
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-04-18 23:56:42
 * @FilePath: /sylar_from_nanasaki/sylar/http/http_utils.h
 */
#pragma once

#include "http_parser.h"
#include "sylar/util/lexical_cast.h"
#include <string>

namespace sylar {

namespace http {

/**
 * @brief HTTP方法枚举
 */
enum class HttpMethod {
#define XX(num, name, string) name = num,
  HTTP_METHOD_MAP(XX)
#undef XX
    INVALID_METHOD
};

/**
 * @brief HTTP状态枚举
 */
enum class HttpStatus {
#define XX(code, name, desc) name = code,
  HTTP_STATUS_MAP(XX)
#undef XX
};

/**
 * @brief 将字符串方法名转成HTTP方法枚举
 * @param[in] m HTTP方法
 * @return HTTP方法枚举
 */
HttpMethod StringToHttpMethod(const std::string& m);

/**
 * @brief 将字符串指针转换成HTTP方法枚举
 * @param[in] m 字符串方法枚举
 * @return HTTP方法枚举
 */
HttpMethod CharsToHttpMethod(const char* m);

/**
 * @brief 将HTTP方法枚举转换成字符串
 * @param[in] m HTTP方法枚举
 * @return 字符串
 */
const char* HttpMethodToString(const HttpMethod& m);

/**
 * @brief 将HTTP状态枚举转换成字符串
 * @param[in] m HTTP状态枚举
 * @return 字符串
 */
const char* HttpStatusToString(const HttpStatus& s);

/**
 * @brief 忽略大小写比较仿函数
 */
struct CaseInsensitiveLess {
  /**
   * @brief 忽略大小写比较字符串
   */
  bool operator()(const std::string& lhs, const std::string& rhs) const;
};

/**
 * @brief 获取Map中的key值,并转成对应类型,返回是否成功
 * @param[in] m Map数据结构
 * @param[in] key 关键字
 * @param[out] val 保存转换后的值
 * @param[in] def 默认值
 * @return
 *      @retval true 转换成功, val 为对应的值
 *      @retval false 不存在或者转换失败 val = def
 */
template <class MapType, class T>
bool checkGetAs(const MapType& m, const std::string& key, T& val, const T& def = T()) {
  auto it = m.find(key);
  if (it == m.end()) {
    val = def;
    return false;
  }
  try {
    val = sylar::util::lexical_cast<T>(it->second);
    return true;
  } catch (...) {
    val = def;
  }
  return false;
}

/**
 * @brief 获取Map中的key值,并转成对应类型
 * @param[in] m Map数据结构
 * @param[in] key 关键字
 * @param[in] def 默认值
 * @return 如果存在且转换成功返回对应的值,否则返回默认值
 */
template <class MapType, class T>
T getAs(const MapType& m, const std::string& key, const T& def = T()) {
  auto it = m.find(key);
  if (it == m.end()) {
    return def;
  }
  try {
    return sylar::util::lexical_cast<T>(it->second);
  } catch (...) {}
  return def;
}


}   // namespace http

}   // namespace sylar
