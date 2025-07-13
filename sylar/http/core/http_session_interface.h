/*
 * @Author: Nana5aki
 * @Date: 2025-07-13 15:00:00
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-07-13 14:39:03
 * @FilePath: /sylar_from_nanasaki/sylar/http/core/http_session_interface.h
 */

#pragma once

#include "http_request.h"
#include "http_response.h"
#include "sylar/socket.h"
#include <memory>
#include <string>

namespace sylar {
namespace http {

/**
 * @brief HTTP会话接口
 * 
 * 提供HTTP请求接收和响应发送的统一接口
 */
class IHttpSession {
public:
  using ptr = std::shared_ptr<IHttpSession>;

  virtual ~IHttpSession() = default;

  /**
   * @brief 接收HTTP请求
   * @return HttpRequest::ptr 接收到的HTTP请求，nullptr表示失败
   */
  virtual HttpRequest::ptr recvRequest() = 0;

  /**
   * @brief 发送HTTP响应
   * @param response HTTP响应对象
   * @return int 发送结果，>0成功，<=0失败
   */
  virtual int sendResponse(HttpResponse::ptr response) = 0;

  /**
   * @brief 关闭会话
   */
  virtual void close() = 0;

  /**
   * @brief 检查会话是否连接
   * @return bool true表示连接有效，false表示连接已断开
   */
  virtual bool isConnected() = 0;

  /**
   * @brief 获取本地地址
   * @return std::string 本地地址
   */
  virtual std::string getLocalAddress() = 0;

  /**
   * @brief 获取远程地址
   * @return std::string 远程地址
   */
  virtual std::string getRemoteAddress() = 0;

  /**
   * @brief 获取底层Socket
   * @return Socket::ptr Socket对象
   */
  virtual Socket::ptr getSocket() = 0;
};

}  // namespace http
}  // namespace sylar 