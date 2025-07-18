/*
 * @Author: Nana5aki
 * @Date: 2025-07-18 00:51:57
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-07-18 22:04:42
 * @FilePath: /sylar_from_nanasaki/sylar/http/https_server.h
 */

#pragma once

#include "servlet.h"
#include "sylar/streams/ssl_socket_stream.h"
#include "sylar/tcp_server.h"
namespace sylar {
namespace http {

class HttpsServer : public TcpServer {
public:
  using ptr = std::shared_ptr<HttpsServer>;

public:
  /**
   * @brief 构造函数
   * @param cert_file 证书文件路径
   * @param key_file 私钥文件路径
   * @param[in] keepalive 是否长连接
   * @param[in] worker 工作调度器
   * @param[in] accept_worker 接收连接调度器
   */
  HttpsServer(const std::string& cert_file, const std::string& key_file, bool keepalive = false,
              sylar::IOManager* worker = sylar::IOManager::GetThis(),
              sylar::IOManager* io_worker = sylar::IOManager::GetThis(),
              sylar::IOManager* accept_worker = sylar::IOManager::GetThis());

  /**
   * @brief 获取ServletDispatch
   */
  ServletDispatch::ptr getServletDispatch() const {
    return m_dispatch;
  }

  /**
   * @brief 设置ServletDispatch
   */
  void setServletDispatch(ServletDispatch::ptr v) {
    m_dispatch = v;
  }

  /**
   * @brief 获取SSL上下文
   */
  SslContext::ptr getSSLContext() const {
    return m_ssl_context;
  }

  virtual void setName(const std::string& v) override;

protected:
  virtual void handleClient(Socket::ptr client) override;

private:
  /**
   * @brief 设置SSL证书和私钥
   * @param cert_file 证书文件路径
   * @param key_file 私钥文件路径
   * @return bool 设置是否成功
   */
  bool setSSLContext(const std::string& cert_file, const std::string& key_file);

private:
  /// 是否支持长连接
  bool m_isKeepalive;
  /// Servlet分发器
  ServletDispatch::ptr m_dispatch;
  /// SSL上下文
  SslContext::ptr m_ssl_context;
};
}   // namespace http
}   // namespace sylar