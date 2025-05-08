/*
 * @Author: Nana5aki
 * @Date: 2025-04-26 22:25:09
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-05-01 19:04:00
 * @FilePath: /MySylar/sylar/http/http_server.h
 */
#pragma once

#include "servlet.h"
#include "sylar/tcp_server.h"
namespace sylar {
namespace http {

class HttpServer : public TcpServer {
public:
  using ptr = std::shared_ptr<HttpServer>;

public:
  /**
   * @brief 构造函数
   * @param[in] keepalive 是否长连接
   * @param[in] worker 工作调度器
   * @param[in] accept_worker 接收连接调度器
   */
  HttpServer(bool keepalive = false, sylar::IOManager* worker = sylar::IOManager::GetThis(),
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

  virtual void setName(const std::string& v) override;

protected:
  virtual void handleClient(Socket::ptr client) override;

private:
  /// 是否支持长连接
  bool m_isKeepalive;
  /// Servlet分发器
  ServletDispatch::ptr m_dispatch;
};
}   // namespace http
}   // namespace sylar