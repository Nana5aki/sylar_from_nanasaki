/*
 * @Author: Nana5aki
 * @Date: 2025-04-26 22:25:09
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-04-27 22:22:51
 * @FilePath: /MySylar/sylar/http/http_server.h
 */
#pragma once

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

  virtual void setName(const std::string& v) override;

protected:
  virtual void handleClient(Socket::ptr client) override;

private:
  /// 是否支持长连接
  bool m_isKeepalive;
};
}   // namespace http
}   // namespace sylar