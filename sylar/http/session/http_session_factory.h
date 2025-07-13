/*
 * @Author: Nana5aki
 * @Date: 2025-07-13 16:00:00
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-07-13 15:15:46
 * @FilePath: /sylar_from_nanasaki/sylar/http/session/http_session_factory.h
 */

#pragma once

#include "sylar/http/core/http_session_interface.h"
#include "sylar/http/core/http_protocol_interface.h"
#include "sylar/http/core/http_transport_interface.h"
#include "sylar/socket.h"
#include <memory>
#include <string>

namespace sylar {
namespace http {

/**
 * @brief HTTP会话工厂
 * 
 * 用于创建不同类型的HTTP会话
 */
class HttpSessionFactory {
public:
  using ptr = std::shared_ptr<HttpSessionFactory>;

  /**
   * @brief 获取单例
   * @return HttpSessionFactory::ptr 工厂实例
   */
  static HttpSessionFactory::ptr getInstance();

  /**
   * @brief 创建HTTP会话
   * @param socket Socket对象
   * @param protocolVersion 协议版本（默认1.1）
   * @param transportType 传输类型（默认tcp）
   * @return HttpSession::ptr HTTP会话对象
   */
  IHttpSession::ptr createSession(Socket::ptr socket,
                                 const std::string& protocolVersion = "1.1",
                                 const std::string& transportType = "tcp");

  /**
   * @brief 创建HTTP会话
   * @param transport 传输层对象
   * @param protocolVersion 协议版本（默认1.1）
   * @return HttpSession::ptr HTTP会话对象
   */
  IHttpSession::ptr createSession(IHttpTransport::ptr transport,
                                 const std::string& protocolVersion = "1.1");

  /**
   * @brief 创建HTTP会话
   * @param protocol 协议处理器
   * @param transport 传输层对象
   * @return HttpSession::ptr HTTP会话对象
   */
  IHttpSession::ptr createSession(IHttpProtocol::ptr protocol,
                                 IHttpTransport::ptr transport);

  /**
   * @brief 创建协议处理器
   * @param version 协议版本
   * @return HttpProtocol::ptr 协议处理器
   */
  IHttpProtocol::ptr createProtocol(const std::string& version);

  /**
   * @brief 创建传输层对象
   * @param socket Socket对象
   * @param type 传输类型
   * @return HttpTransport::ptr 传输层对象
   */
  IHttpTransport::ptr createTransport(Socket::ptr socket, const std::string& type);

private:
  HttpSessionFactory() = default;
  static HttpSessionFactory::ptr s_instance;
};

}  // namespace http
}  // namespace sylar 