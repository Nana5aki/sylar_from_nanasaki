/*
 * @Author: Nana5aki
 * @Date: 2025-04-19 00:04:48
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-05-24 23:28:44
 * @FilePath: /sylar_from_nanasaki/sylar/http/http_response.h
 */
#pragma once

#include "sylar/http/http_utils.h"
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace sylar {
namespace http {
class HttpResponse {
public:
  using ptr = std::shared_ptr<HttpResponse>;
  using MapType = std::map<std::string, std::string, CaseInsensitiveLess>;

public:
  /**
   * @brief 构造函数
   * @param[in] version 版本
   * @param[in] close 是否自动关闭
   */
  HttpResponse(uint8_t version = 0x11, bool close = true);

  /**
   * @brief 返回响应状态
   * @return 请求状态
   */
  HttpStatus getStatus() const {
    return m_status;
  }

  /**
   * @brief 返回响应版本
   * @return 版本
   */
  uint8_t getVersion() const {
    return m_version;
  }

  /**
   * @brief 返回响应消息体
   * @return 消息体
   */
  const std::string& getBody() const {
    return m_body;
  }

  /**
   * @brief 返回响应原因
   */
  const std::string& getReason() const {
    return m_reason;
  }

  /**
   * @brief 返回响应头部MAP
   * @return MAP
   */
  const MapType& getHeaders() const {
    return m_headers;
  }

  /**
   * @brief 设置响应状态
   * @param[in] v 响应状态
   */
  void setStatus(HttpStatus v) {
    m_status = v;
  }

  /**
   * @brief 设置响应版本
   * @param[in] v 版本
   */
  void setVersion(uint8_t v) {
    m_version = v;
  }

  /**
   * @brief 设置响应消息体
   * @param[in] v 消息体
   */
  void setBody(const std::string& v) {
    m_body = v;
  }

  /**
   * @brief 追加HTTP请求的消息体
   * @param[in] v 追加内容
   */
  void appendBody(const std::string& v) {
    m_body.append(v);
  }

  /**
   * @brief 设置响应原因
   * @param[in] v 原因
   */
  void setReason(const std::string& v) {
    m_reason = v;
  }

  /**
   * @brief 设置响应头部MAP
   * @param[in] v MAP
   */
  void setHeaders(const MapType& v) {
    m_headers = v;
  }

  /**
   * @brief 是否自动关闭
   */
  bool isClose() const {
    return m_close;
  }

  /**
   * @brief 设置是否自动关闭
   */
  void setClose(bool v) {
    m_close = v;
  }

  /**
   * @brief 是否websocket
   */
  bool isWebsocket() const {
    return m_websocket;
  }

  /**
   * @brief 设置是否websocket
   */
  void setWebsocket(bool v) {
    m_websocket = v;
  }

  /**
   * @brief 获取响应头部参数
   * @param[in] key 关键字
   * @param[in] def 默认值
   * @return 如果存在返回对应值,否则返回def
   */
  std::string getHeader(const std::string& key, const std::string& def = "") const;

  /**
   * @brief 设置响应头部参数
   * @param[in] key 关键字
   * @param[in] val 值
   */
  void setHeader(const std::string& key, const std::string& val);

  /**
   * @brief 删除响应头部参数
   * @param[in] key 关键字
   */
  void delHeader(const std::string& key);

  /**
   * @brief 检查并获取响应头部参数
   * @tparam T 值类型
   * @param[in] key 关键字
   * @param[out] val 值
   * @param[in] def 默认值
   * @return 如果存在且转换成功返回true,否则失败val=def
   */
  template <class T>
  bool checkGetHeaderAs(const std::string& key, T& val, const T& def = T()) {
    return checkGetAs(m_headers, key, val, def);
  }

  /**
   * @brief 获取响应的头部参数
   * @tparam T 转换类型
   * @param[in] key 关键字
   * @param[in] def 默认值
   * @return 如果存在且转换成功返回对应的值,否则返回def
   */
  template <class T>
  T getHeaderAs(const std::string& key, const T& def = T()) {
    return getAs(m_headers, key, def);
  }

  /**
   * @brief 序列化输出到流
   * @param[in, out] os 输出流
   * @return 输出流
   */
  std::ostream& dump(std::ostream& os) const;

  /**
   * @brief 转成字符串
   */
  std::string toString() const;

  /**
   * @brief 设置重定向，在头部添加Location字段，值为uri
   * @param[] uri 目标uri
   */
  void setRedirect(const std::string& uri);

  /**
   * @brief 为响应添加cookie
   * @param[] key cookie的key值
   * @param[] val cookie的value
   * @param[] expired 过期时间
   * @param[] path cookie的影响路径
   * @param[] domain cookie作用的域
   * @param[] secure 安全标志
   */
  void setCookie(const std::string& key, const std::string& val, time_t expired = 0,
                 const std::string& path = "", const std::string& domain = "", bool secure = false);

private:
  /// 响应状态
  HttpStatus m_status;
  /// 版本
  uint8_t m_version;
  /// 是否自动关闭
  bool m_close;
  /// 是否为websocket
  bool m_websocket;
  /// 响应消息体
  std::string m_body;
  /// 响应原因
  std::string m_reason;
  /// 响应头部MAP
  MapType m_headers;
  /// cookies
  std::vector<std::string> m_cookies;
};

/**
 * @brief 流式输出HttpResponse
 * @param[in, out] os 输出流
 * @param[in] rsp HTTP响应
 * @return 输出流
 */
std::ostream& operator<<(std::ostream& os, const HttpResponse& rsp);

}   // namespace http
}   // namespace sylar