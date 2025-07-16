/*
 * @Author: Nana5aki
 * @Date: 2025-07-16 01:13:13
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-07-16 23:51:07
 * @FilePath: /sylar_from_nanasaki/sylar/http/transport/ssl_transport.h
 */

#pragma once

#include "sylar/http/core/http_transport_interface.h"
#include "sylar/streams/ssl_socket_stream.h"
#include <memory>
#include <string>

namespace sylar {
namespace http {

/**
 * @brief SSL传输层实现
 */
class SslTransport : public IHttpTransport {
public:
  using ptr = std::shared_ptr<SslTransport>;

  /**
   * @brief 构造函数
   * @param ssl_stream SSL套接字流
   */
  SslTransport(SslSocketStream::ptr ssl_stream);
  
  /**
   * @brief 构造函数 - 从Socket和SSL上下文创建
   * @param socket TCP套接字
   * @param ssl_context SSL上下文
   * @param owner_socket 是否拥有Socket
   */
  SslTransport(Socket::ptr socket, SslContext::ptr ssl_context, bool owner_socket = true);

  virtual ~SslTransport() = default;

  /**
   * @brief 读取数据
   * @param buffer 缓冲区
   * @param length 缓冲区大小
   * @return int 读取的字节数，<=0表示错误或连接关闭
   */
  virtual int read(void* buffer, size_t length) override;

  /**
   * @brief 写入数据
   * @param buffer 数据缓冲区
   * @param length 数据长度
   * @return int 写入的字节数，<=0表示错误或连接关闭
   */
  virtual int write(const void* buffer, size_t length) override;

  /**
   * @brief 写入固定长度数据
   * @param buffer 数据缓冲区
   * @param length 数据长度
   * @return int 写入结果，<=0表示错误
   */
  virtual int writeFixSize(const void* buffer, size_t length) override;

  /**
   * @brief 关闭传输连接
   */
  virtual void close() override;

  /**
   * @brief 检查连接状态
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
   * @brief 执行SSL握手
   * @return bool 握手是否成功
   */
  bool doHandshake();

  /**
   * @brief 检查SSL连接是否有效
   * @return bool 连接是否有效
   */
  bool isSSLValid() const;

  /**
   * @brief 获取SSL连接信息
   * @return std::string SSL连接信息字符串
   */
  std::string getSSLInfo() const;

  /**
   * @brief 获取使用的密码套件
   * @return std::string 密码套件名称
   */
  std::string getCipherSuite() const;

  /**
   * @brief 获取TLS版本
   * @return std::string TLS版本字符串
   */
  std::string getTLSVersion() const;

  /**
   * @brief 获取SslSocketStream
   * @return SslSocketStream::ptr SslSocketStream对象
   */
  SslSocketStream::ptr getSslSocketStream() const;

private:
  /// SSL套接字流
  SslSocketStream::ptr m_ssl_stream;
  /// 是否已完成握手
  bool m_handshake_done;
};

}   // namespace http
}   // namespace sylar