/*
 * @Author: Nana5aki
 * @Date: 2025-07-13 15:10:00
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-07-13 15:02:53
 * @FilePath: /sylar_from_nanasaki/sylar/http/core/http_protocol_interface.h
 */

#pragma once

#include "http_transport_interface.h"
#include "http_request.h"
#include "http_response.h"
#include <memory>
#include <string>

namespace sylar {
namespace http {

/**
 * @brief HTTP协议处理器接口
 */
class IHttpProtocol {
public:
  using ptr = std::shared_ptr<IHttpProtocol>;

  virtual ~IHttpProtocol() = default;

  /**
   * @brief 获取协议版本
   * @return std::string 协议版本字符串
   */
  virtual std::string getVersion() = 0;

  /**
   * @brief 接收HTTP请求
   * @param transport 传输层对象
   * @return HttpRequest::ptr 解析后的HTTP请求，nullptr表示失败
   */
  virtual HttpRequest::ptr recvRequest(IHttpTransport::ptr transport) = 0;

  /**
   * @brief 发送HTTP响应
   * @param transport 传输层对象
   * @param response HTTP响应对象
   * @return int 发送结果，>0成功，<=0失败
   */
  virtual int sendResponse(IHttpTransport::ptr transport, HttpResponse::ptr response) = 0;

  /**
   * @brief 检查是否支持Keep-Alive
   * @return bool true支持，false不支持
   */
  virtual bool supportsKeepAlive() = 0;
};

}   // namespace http
}   // namespace sylar