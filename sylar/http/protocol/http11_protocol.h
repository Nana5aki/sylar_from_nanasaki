/*
 * @Author: Nana5aki
 * @Date: 2025-07-13 15:25:00
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-07-13 15:03:11
 * @FilePath: /sylar_from_nanasaki/sylar/http/protocol/http11_protocol.h
 */

#pragma once

#include "sylar/http/core/http_protocol_interface.h"
#include <memory>

namespace sylar {
namespace http {

/**
 * @brief HTTP/1.1协议处理器
 * 
 * 实现HTTP/1.1协议的请求接收和响应发送
 */
class Http11Protocol : public IHttpProtocol {
public:
  using ptr = std::shared_ptr<Http11Protocol>;

  Http11Protocol();
  virtual ~Http11Protocol() = default;

  /**
   * @brief 获取协议版本
   * @return std::string 固定返回"1.1"
   */
  virtual std::string getVersion() override;

  /**
   * @brief 接收HTTP请求
   * @param transport 传输层对象
   * @return HttpRequest::ptr 解析后的HTTP请求，nullptr表示失败
   */
  virtual HttpRequest::ptr recvRequest(IHttpTransport::ptr transport) override;

  /**
   * @brief 发送HTTP响应
   * @param transport 传输层对象
   * @param response HTTP响应对象
   * @return int 发送结果，>0成功，<=0失败
   */
  virtual int sendResponse(IHttpTransport::ptr transport, HttpResponse::ptr response) override;

  /**
   * @brief 检查是否支持Keep-Alive
   * @return bool true支持
   */
  virtual bool supportsKeepAlive() override;

private:
  /// 获取HTTP请求缓冲区大小
  uint64_t getHttpRequestBufferSize();
};

}  // namespace http
}  // namespace sylar 