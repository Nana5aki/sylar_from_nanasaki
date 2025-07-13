/*
 * @Author: Nana5aki
 * @Date: 2025-07-13 14:21:26
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-07-13 15:13:38
 * @FilePath: /sylar_from_nanasaki/sylar/http/session/http_session_impl.h
 */
/*
 * @Author: Nana5aki
 * @Date: 2025-07-13 15:35:00
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-07-13 15:35:00
 * @FilePath: /sylar_from_nanasaki/sylar/http/session/http_session_impl.h
 */

#pragma once

#include "sylar/http/core/http_protocol_interface.h"
#include "sylar/http/core/http_session_interface.h"
#include "sylar/http/core/http_transport_interface.h"
#include <memory>

namespace sylar {
namespace http {

/**
 * @brief HTTP会话实现类
 *
 * 基于协议层和传输层的HTTP会话实现
 */
class HttpSessionImpl : public IHttpSession {
public:
  using ptr = std::shared_ptr<HttpSessionImpl>;

  /**
   * @brief 构造函数
   * @param protocol 协议处理器
   * @param transport 传输层对象
   */
  HttpSessionImpl(IHttpProtocol::ptr protocol, IHttpTransport::ptr transport);

  virtual ~HttpSessionImpl() = default;

  /**
   * @brief 接收HTTP请求
   * @return HttpRequest::ptr 接收到的HTTP请求，nullptr表示失败
   */
  virtual HttpRequest::ptr recvRequest() override;

  /**
   * @brief 发送HTTP响应
   * @param response HTTP响应对象
   * @return int 发送结果，>0成功，<=0失败
   */
  virtual int sendResponse(HttpResponse::ptr response) override;

  /**
   * @brief 关闭会话
   */
  virtual void close() override;

  /**
   * @brief 检查会话是否连接
   * @return bool true表示连接有效，false表示连接已断开
   */
  virtual bool isConnected() override;

  /**
   * @brief 获取本地地址
   * @return std::string 本地地址
   */
  virtual std::string getLocalAddress() override;

  /**
   * @brief 获取远程地址
   * @return std::string 远程地址
   */
  virtual std::string getRemoteAddress() override;

  /**
   * @brief 获取底层Socket
   * @return Socket::ptr Socket对象
   */
  virtual Socket::ptr getSocket() override;

  /**
   * @brief 获取协议处理器
   * @return HttpProtocol::ptr 协议处理器
   */
  IHttpProtocol::ptr getProtocol() const {
    return m_protocol;
  }
  /**
   * @brief 获取传输层对象
   * @return HttpTransport::ptr 传输层对象
   */
  IHttpTransport::ptr getTransport() const {
    return m_transport;
  }

private:
  /// 协议处理器
  IHttpProtocol::ptr m_protocol;
  /// 传输层对象
  IHttpTransport::ptr m_transport;
};

}   // namespace http
}   // namespace sylar