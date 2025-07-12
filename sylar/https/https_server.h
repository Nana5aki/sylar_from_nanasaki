/*
 * @Author: Nana5aki
 * @Date: 2025-01-02 10:00:00
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-07-13 00:18:17
 * @FilePath: /sylar_from_nanasaki/sylar/https/https_server.h
 * @Description: HTTPS服务器头文件 - 基于TcpServer扩展SSL/TLS支持
 *               提供完整的HTTPS服务器功能，支持SSL证书配置和客户端连接管理
 */
#pragma once

#include "sylar/http/servlet.h"
#include "sylar/streams/ssl_socket_stream.h"
#include "sylar/tcp_server.h"
#include <memory>
#include <openssl/ssl.h>

namespace sylar {
namespace https {

/**
 * @brief HTTPS服务器类
 * @details 继承自TcpServer，在TCP服务器基础上添加SSL/TLS加密支持
 *          提供完整的HTTPS服务器功能，包括SSL证书管理、客户端连接处理等
 *          集成Servlet分发器，支持灵活的HTTP请求路由和处理
 *
 * 主要功能：
 * - SSL证书配置和管理
 * - HTTPS客户端连接接受和处理
 * - SSL握手管理
 * - HTTP请求路由和处理（通过ServletDispatch）
 * - 支持多种SSL配置选项
 * - 长连接支持
 *
 * 使用场景：
 * - Web服务器的HTTPS支持
 * - 需要加密传输的HTTP API服务
 * - 安全的Web应用服务器
 * - 需要客户端证书验证的安全服务
 */
class HttpsServer : public TcpServer {
public:
  using ptr = std::shared_ptr<HttpsServer>;

  /**
   * @brief 构造函数 - 创建HTTPS服务器实例
   * @param[in] keepalive 是否支持长连接 - true启用HTTP Keep-Alive
   * @param[in] worker 工作调度器 - 处理业务逻辑的协程调度器
   * @param[in] io_worker IO调度器 - 处理网络IO的协程调度器
   * @param[in] accept_worker 接收连接调度器 - 处理客户端连接接收的协程调度器
   * @details 初始化HTTPS服务器，设置各种调度器和基本参数
   *          默认使用当前IOManager作为所有调度器
   */
  HttpsServer(bool keepalive = false, sylar::IOManager* worker = sylar::IOManager::GetThis(),
              sylar::IOManager* io_worker = sylar::IOManager::GetThis(),
              sylar::IOManager* accept_worker = sylar::IOManager::GetThis());

  /**
   * @brief 析构函数 - 清理HTTPS服务器资源
   * @details 停止服务器、清理SSL上下文、关闭所有连接
   */
  virtual ~HttpsServer();

  /**
   * @brief 获取Servlet分发器
   * @return sylar::http::ServletDispatch::ptr HTTP请求分发器智能指针
   * @details 返回用于HTTP请求路由和处理的Servlet分发器
   *          可用于添加自定义的HTTP请求处理器
   */
  sylar::http::ServletDispatch::ptr getServletDispatch() const {
    return m_dispatch;
  }

  /**
   * @brief 设置Servlet分发器
   * @param[in] v HTTP请求分发器智能指针
   * @details 设置用于HTTP请求路由和处理的Servlet分发器
   *          允许自定义HTTP请求的处理逻辑
   */
  void setServletDispatch(sylar::http::ServletDispatch::ptr v) {
    m_dispatch = v;
  }

  /**
   * @brief 设置服务器名称
   * @param[in] v 服务器名称字符串
   * @details 设置HTTPS服务器的名称，用于日志记录和状态显示
   */
  virtual void setName(const std::string& v) override;

  /**
   * @brief 加载SSL证书文件
   * @param[in] cert_file 证书文件路径 - PEM格式的服务器证书文件
   * @param[in] key_file 私钥文件路径 - PEM格式的服务器私钥文件
   * @return bool 是否加载成功
   *         true表示证书和私钥加载成功且匹配
   *         false表示文件不存在、格式错误或私钥与证书不匹配
   */
  bool loadCertificates(const std::string& cert_file, const std::string& key_file);

  /**
   * @brief 设置SSL上下文
   * @param[in] ctx SSL上下文
   */
  void setSslContext(SslContext::ptr ctx) {
    m_ssl_ctx = ctx;
  }

  /**
   * @brief 获取SSL上下文
   */
  SslContext::ptr getSslContext() const {
    return m_ssl_ctx;
  }

  /**
   * @brief 设置SSL验证模式
   * @param[in] mode 验证模式
   */
  void setVerifyMode(int mode);

  /**
   * @brief 设置SSL验证回调
   * @param[in] callback 回调函数
   */
  void setVerifyCallback(int (*callback)(int, X509_STORE_CTX*));

  /**
   * @brief 设置密码回调
   * @param[in] callback 回调函数
   */
  void setPasswordCallback(int (*callback)(char*, int, int, void*));

  /**
   * @brief 初始化SSL
   * @return 是否成功
   */
  bool initSSL();

  /**
   * @brief 启动服务器
   * @return 是否成功
   */
  virtual bool start() override;

  /**
   * @brief 停止服务器
   */
  virtual void stop() override;

  /**
   * @brief 获取服务器状态信息
   * @return 状态信息字符串
   */
  virtual std::string toString(const std::string& prefix = "") override;

protected:
  /**
   * @brief 处理客户端连接
   * @param[in] client 客户端socket
   */
  virtual void handleClient(Socket::ptr client) override;

  /**
   * @brief 初始化SSL库
   */
  void initSSLLibrary();

  /**
   * @brief 清理SSL库
   */
  void cleanupSSLLibrary();

private:
  /// 是否支持长连接
  bool m_is_keepalive;
  /// Servlet分发器
  sylar::http::ServletDispatch::ptr m_dispatch;
  /// SSL上下文
  SslContext::ptr m_ssl_ctx;
  /// 证书文件路径
  std::string m_cert_file;
  /// 私钥文件路径
  std::string m_key_file;
  /// SSL是否已初始化
  bool m_ssl_initialized;
  /// SSL验证模式
  int m_verify_mode;
  /// SSL验证回调
  int (*m_verify_callback)(int, X509_STORE_CTX*);
  /// 密码回调
  int (*m_password_callback)(char*, int, int, void*);
};

}   // namespace https
}   // namespace sylar