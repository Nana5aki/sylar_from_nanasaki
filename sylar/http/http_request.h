/*
 * @Author: Nana5aki
 * @Date: 2025-04-19 00:02:57
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-04-29 22:53:49
 * @FilePath: /sylar_from_nanasaki/sylar/http/http_request.h
 */
#pragma once

#include "http_response.h"
#include "http_utils.h"
#include <cstdint>
#include <map>
#include <memory>
#include <string>

namespace sylar {
namespace http {

/// @brief HTTP请求
class HttpRequest {
public:
  using ptr = std::shared_ptr<HttpRequest>;
  using MapType = std::map<std::string, std::string, CaseInsensitiveLess>;

public:
  /**
   * @brief 构造函数
   * @param[in] version 版本
   * @param[in] close 是否keepalive
   */
  HttpRequest(uint8_t version = 0x11, bool close = true);

  /**
   * @brief 从HTTP请求构造HTTP响应
   * @note 只需要保证请求与响应的版本号与keep-alive一致即可
   */
  std::shared_ptr<HttpResponse> createResponse();

  /**
   * @brief 返回HTTP方法
   */
  HttpMethod getMethod() const {
    return m_method;
  }

  /**
   * @brief 返回HTTP版本
   */
  uint8_t getVersion() const {
    return m_version;
  }

  /**
   * @brief 返回HTTP请求的路径
   */
  const std::string& getPath() const {
    return m_path;
  }

  /**
   * @brief 返回HTTP请求的查询参数
   */
  const std::string& getQuery() const {
    return m_query;
  }

  /**
   * @brief 返回HTTP请求的消息体
   */
  const std::string& getBody() const {
    return m_body;
  }

  /**
   * @brief 返回HTTP请求的消息头MAP
   */
  const MapType& getHeaders() const {
    return m_headers;
  }

  /**
   * @brief 返回HTTP请求的参数MAP
   */
  const MapType& getParams() const {
    return m_params;
  }

  /**
   * @brief 返回HTTP请求的cookie MAP
   */
  const MapType& getCookies() const {
    return m_cookies;
  }

  /**
   * @brief 设置HTTP请求的方法名
   * @param[in] v HTTP请求
   */
  void setMethod(HttpMethod v) {
    m_method = v;
  }

  /**
   * @brief 设置HTTP请求的协议版本
   * @param[in] v 协议版本0x11, 0x10
   */
  void setVersion(uint8_t v) {
    m_version = v;
  }

  /**
   * @brief 设置HTTP请求的路径
   * @param[in] v 请求路径
   */
  void setPath(const std::string& v) {
    m_path = v;
  }

  /**
   * @brief 设置HTTP请求的查询参数
   * @param[in] v 查询参数
   */
  void setQuery(const std::string& v) {
    m_query = v;
  }

  /**
   * @brief 设置HTTP请求的Fragment
   * @param[in] v fragment
   */
  void setFragment(const std::string& v) {
    m_fragment = v;
  }

  /**
   * @brief 设置HTTP请求的消息体
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
   * @brief 设置HTTP请求的头部MAP
   * @param[in] v map
   */
  void setHeaders(const MapType& v) {
    m_headers = v;
  }

  /**
   * @brief 设置HTTP请求的参数MAP
   * @param[in] v map
   */
  void setParams(const MapType& v) {
    m_params = v;
  }

  /**
   * @brief 设置HTTP请求的Cookie MAP
   * @param[in] v map
   */
  void setCookies(const MapType& v) {
    m_cookies = v;
  }

  /**
   * @brief 获取HTTP请求的头部参数
   * @param[in] key 关键字
   * @param[in] def 默认值
   * @return 如果存在则返回对应值,否则返回默认值
   */
  std::string getHeader(const std::string& key, const std::string& def = "") const;

  /**
   * @brief 获取HTTP请求的请求参数
   * @param[in] key 关键字
   * @param[in] def 默认值
   * @return 如果存在则返回对应值,否则返回默认值
   */
  std::string getParam(const std::string& key, const std::string& def = "");

  /**
   * @brief 获取HTTP请求的Cookie参数
   * @param[in] key 关键字
   * @param[in] def 默认值
   * @return 如果存在则返回对应值,否则返回默认值
   */
  std::string getCookie(const std::string& key, const std::string& def = "");


  /**
   * @brief 设置HTTP请求的头部参数
   * @param[in] key 关键字
   * @param[in] val 值
   */
  void setHeader(const std::string& key, const std::string& val);

  /**
   * @brief 设置HTTP请求的请求参数
   * @param[in] key 关键字
   * @param[in] val 值
   */

  void setParam(const std::string& key, const std::string& val);
  /**
   * @brief 设置HTTP请求的Cookie参数
   * @param[in] key 关键字
   * @param[in] val 值
   */
  void setCookie(const std::string& key, const std::string& val);

  /**
   * @brief 删除HTTP请求的头部参数
   * @param[in] key 关键字
   */
  void delHeader(const std::string& key);

  /**
   * @brief 删除HTTP请求的请求参数
   * @param[in] key 关键字
   */
  void delParam(const std::string& key);

  /**
   * @brief 删除HTTP请求的Cookie参数
   * @param[in] key 关键字
   */
  void delCookie(const std::string& key);

  /**
   * @brief 判断HTTP请求的头部参数是否存在
   * @param[in] key 关键字
   * @param[out] val 如果存在,val非空则赋值
   * @return 是否存在
   */
  bool hasHeader(const std::string& key, std::string* val = nullptr);

  /**
   * @brief 判断HTTP请求的请求参数是否存在
   * @param[in] key 关键字
   * @param[out] val 如果存在,val非空则赋值
   * @return 是否存在
   */
  bool hasParam(const std::string& key, std::string* val = nullptr);

  /**
   * @brief 判断HTTP请求的Cookie参数是否存在
   * @param[in] key 关键字
   * @param[out] val 如果存在,val非空则赋值
   * @return 是否存在
   */
  bool hasCookie(const std::string& key, std::string* val = nullptr);

  /**
   * @brief 检查并获取HTTP请求的头部参数
   * @tparam T 转换类型
   * @param[in] key 关键字
   * @param[out] val 返回值
   * @param[in] def 默认值
   * @return 如果存在且转换成功返回true,否则失败val=def
   */
  template <class T>
  bool checkGetHeaderAs(const std::string& key, T& val, const T& def = T()) {
    return checkGetAs(m_headers, key, val, def);
  }

  /**
   * @brief 获取HTTP请求的头部参数
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
   * @brief 检查并获取HTTP请求的请求参数
   * @tparam T 转换类型
   * @param[in] key 关键字
   * @param[out] val 返回值
   * @param[in] def 默认值
   * @return 如果存在且转换成功返回true,否则失败val=def
   */
  template <class T>
  bool checkGetParamAs(const std::string& key, T& val, const T& def = T()) {
    initQueryParam();
    initBodyParam();
    return checkGetAs(m_params, key, val, def);
  }

  /**
   * @brief 获取HTTP请求的请求参数
   * @tparam T 转换类型
   * @param[in] key 关键字
   * @param[in] def 默认值
   * @return 如果存在且转换成功返回对应的值,否则返回def
   */
  template <class T>
  T getParamAs(const std::string& key, const T& def = T()) {
    initQueryParam();
    initBodyParam();
    return getAs(m_params, key, def);
  }

  /**
   * @brief 检查并获取HTTP请求的Cookie参数
   * @tparam T 转换类型
   * @param[in] key 关键字
   * @param[out] val 返回值
   * @param[in] def 默认值
   * @return 如果存在且转换成功返回true,否则失败val=def
   */
  template <class T>
  bool checkGetCookieAs(const std::string& key, T& val, const T& def = T()) {
    initCookies();
    return checkGetAs(m_cookies, key, val, def);
  }

  /**
   * @brief 获取HTTP请求的Cookie参数
   * @tparam T 转换类型
   * @param[in] key 关键字
   * @param[in] def 默认值
   * @return 如果存在且转换成功返回对应的值,否则返回def
   */
  template <class T>
  T getCookieAs(const std::string& key, const T& def = T()) {
    initCookies();
    return getAs(m_cookies, key, def);
  }

  /**
   * @brief 序列化输出到流中
   * @param[in, out] os 输出流
   * @return 输出流
   */
  std::ostream& dump(std::ostream& os) const;

  /**
   * @brief 转成字符串类型
   * @return 字符串
   */
  std::string toString() const;

  /**
   * @brief 提取url中的查询参数
   */
  void initQueryParam();

  /**
   * @brief 当content-type是application/x-www-form-urlencoded时，提取消息体中的表单参数
   */
  void initBodyParam();

  /**
   * @brief 提取请求中的cookies
   */
  void initCookies();

  /**
   * @brief 初始化，实际是判断connection是否为keep-alive，以设置是否自动关闭套接字
   */
  void init();

private:
  /// HTTP方法
  HttpMethod m_method;
  /// HTTP版本
  uint8_t m_version;
  /// 是否自动关闭
  bool m_close;
  /// 是否为websocket
  bool m_websocket;
  /// 参数解析标志位，0:未解析，1:已解析url参数, 2:已解析http消息体中的参数，4:已解析cookies
  uint8_t m_parserParamFlag;
  /// 请求的完整url
  std::string m_url;
  /// 请求路径
  std::string m_path;
  /// 请求参数
  std::string m_query;
  /// 请求fragment
  std::string m_fragment;
  /// 请求消息体
  std::string m_body;
  /// 请求头部MAP
  MapType m_headers;
  /// 请求参数MAP
  MapType m_params;
  /// 请求Cookie MAP
  MapType m_cookies;
};

/**
 * @brief 流式输出HttpRequest
 * @param[in, out] os 输出流
 * @param[in] req HTTP请求
 * @return 输出流
 */
std::ostream& operator<<(std::ostream& os, const HttpRequest& req);

}   // namespace http
}   // namespace sylar