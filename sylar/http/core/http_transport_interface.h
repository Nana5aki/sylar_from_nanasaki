/*
 * @Author: Nana5aki
 * @Date: 2025-07-13 15:05:00
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-07-13 14:56:02
 * @FilePath: /sylar_from_nanasaki/sylar/http/core/http_transport_interface.h
 */

#pragma once

#include "sylar/socket.h"
#include <memory>
#include <string>

namespace sylar {
namespace http {

/**
 * @brief HTTP传输层接口
 * 
 * 抽象不同的传输层实现（TCP、SSL等）
 */
class IHttpTransport {
public:
  using ptr = std::shared_ptr<IHttpTransport>;

  virtual ~IHttpTransport() = default;

  /**
   * @brief 读取数据
   * @param buffer 缓冲区
   * @param length 缓冲区大小
   * @return int 读取的字节数，<=0表示错误或连接关闭
   */
  virtual int read(void* buffer, size_t length) = 0;

  /**
   * @brief 写入数据
   * @param buffer 数据缓冲区
   * @param length 数据长度
   * @return int 写入的字节数，<=0表示错误或连接关闭
   */
  virtual int write(const void* buffer, size_t length) = 0;

  /**
   * @brief 写入固定长度数据
   * @param buffer 数据缓冲区
   * @param length 数据长度
   * @return int 写入结果，<=0表示错误
   */
  virtual int writeFixSize(const void* buffer, size_t length) = 0;

  /**
   * @brief 关闭传输连接
   */
  virtual void close() = 0;

  /**
   * @brief 检查连接状态
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