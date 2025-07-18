/*
 * @Author: Nana5aki
 * @Date: 2025-07-13 16:00:00
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-07-18 22:58:21
 * @FilePath: /sylar_from_nanasaki/sylar/http/session/http_session_factory.h
 */

#pragma once

#include "sylar/http/core/http_protocol_interface.h"
#include "sylar/http/core/http_session_interface.h"
#include "sylar/socket.h"
#include "sylar/streams/ssl_socket_stream.h"
#include <memory>

namespace sylar {
namespace http {

enum class HttpProtocolType {
  HTTP1_1,
  HTTP2,
  HTTP3,
};

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
  IHttpSession::ptr createHttpSession(
    Socket::ptr socket, const HttpProtocolType& protocolVersion = HttpProtocolType::HTTP1_1);

  IHttpSession::ptr createHttpsSession(
    Socket::ptr socket, SslContext::ptr ssl_context,
    const HttpProtocolType& protocolVersion = HttpProtocolType::HTTP1_1);

private:
  /**
   * @brief 创建协议处理器
   * @param version 协议版本
   * @return HttpProtocol::ptr 协议处理器
   */
  IHttpProtocol::ptr createProtocol(const HttpProtocolType& protocolVersion);

private:
  HttpSessionFactory() = default;
  static HttpSessionFactory::ptr s_instance;
};

}   // namespace http
}   // namespace sylar