/*
 * @Author: Nana5aki
 * @Date: 2025-04-20 18:29:00
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-04-20 18:50:37
 * @FilePath: /MySylar/sylar/http/http_session.h
 */
#pragma once

#include "http_request.h"
#include "http_response.h"
#include "sylar/socket.h"
#include "sylar/streams/sock_stream.h"

namespace sylar {
namespace http {

/**
 * @brief HTTPSession封装
 */
class HttpSession : public SocketStream {
public:
  /// 智能指针类型定义
  using ptr = std::shared_ptr<HttpSession>;

public:
  /**
   * @brief 构造函数
   * @param[in] sock Socket类型
   * @param[in] owner 是否托管
   */
  HttpSession(Socket::ptr sock, bool owner = true);

  /**
   * @brief 接收HTTP请求
   */
  HttpRequest::ptr recvRequest();

  /**
   * @brief 发送HTTP响应
   * @param[in] rsp HTTP响应
   * @return >0 发送成功
   *         =0 对方关闭
   *         <0 Socket异常
   */
  int sendResponse(HttpResponse::ptr rsp);
};

}   // namespace http
}   // namespace sylar
