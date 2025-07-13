/*
 * @Author: Nana5aki
 * @Date: 2025-07-13 15:15:00
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-07-13 14:55:43
 * @FilePath: /sylar_from_nanasaki/sylar/http/transport/tcp_transport.h
 */

#pragma once

#include "sylar/http/core/http_transport_interface.h"
#include "sylar/streams/sock_stream.h"
#include <memory>

namespace sylar {
namespace http {

/**
 * @brief TCP传输层实现
 * 
 * 基于SocketStream的TCP传输层实现
 */
class TcpTransport : public IHttpTransport {
public:
  using ptr = std::shared_ptr<TcpTransport>;

  /**
   * @brief 构造函数
   * @param socket Socket对象
   * @param owner 是否拥有socket
   */
  TcpTransport(Socket::ptr socket, bool owner = true);

  /**
   * @brief 构造函数
   * @param sockStream SocketStream对象
   */
  TcpTransport(SocketStream::ptr sockStream);

  virtual ~TcpTransport() = default;

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
   * @brief 获取SocketStream
   * @return SocketStream::ptr SocketStream对象
   */
  SocketStream::ptr getSocketStream();

private:
  /// SocketStream对象
  SocketStream::ptr m_sockStream;
};

}  // namespace http
}  // namespace sylar 