/*
 * @Author: Nana5aki
 * @Date: 2025-01-02 10:00:00
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-07-12 18:00:57
 * @FilePath: /sylar_from_nanasaki/sylar/https/https_session.h
 * @Description: HTTPS会话头文件 - 基于HttpSession扩展SSL/TLS支持
 *               提供HTTPS服务器端会话处理能力，支持加密HTTP通信
 */
#pragma once

#include "sylar/http/http_session.h"
#include "sylar/streams/ssl_socket_stream.h"
#include "sylar/http/http_request.h"
#include "sylar/http/http_response.h"
#include "sylar/socket.h"
#include <openssl/ssl.h>
#include <openssl/x509.h>

namespace sylar {
namespace https {

/**
 * @brief HTTPS会话封装类
 * @details 继承自HttpSession，在HTTP会话基础上添加SSL/TLS加密支持
 *          复用HttpSession的HTTP协议处理逻辑，专注于SSL层面的功能
 *          提供完整的HTTPS会话管理功能
 */
class HttpsSession : public sylar::http::HttpSession {
public:
    using ptr = std::shared_ptr<HttpsSession>;

      /**
   * @brief 构造函数 - 使用已存在的SSL对象
   * @param[in] sock Socket类型 - 底层TCP套接字
   * @param[in] ssl SSL连接对象 - 已创建的SSL连接对象
   * @param[in] owner 是否托管socket所有权 - true表示析构时关闭socket
   * @details 适用于已经完成SSL握手的连接，通常用于服务器端接受连接后创建会话
   *          SSL对象应该已经与socket关联并可能已完成握手
   */
  HttpsSession(Socket::ptr sock, SSL* ssl, bool owner = true);

  /**
   * @brief 构造函数 - 使用SSL上下文创建新连接
   * @param[in] sock Socket类型 - 底层TCP套接字
   * @param[in] ctx SSL上下文 - SSL配置上下文，用于创建SSL对象
   * @param[in] owner 是否托管socket所有权 - true表示析构时关闭socket
   * @details 会基于SSL上下文创建新的SSL对象，适用于需要动态创建SSL连接的场景
   *          SSL握手需要在创建后单独调用doHandshake()方法完成
   */
  HttpsSession(Socket::ptr sock, SslContext::ptr ctx, bool owner = true);

  /**
   * @brief 析构函数
   * @details 清理HTTPS会话相关资源，包括SSL对象和socket的清理
   */
  ~HttpsSession();

      /**
   * @brief 重写读取方法 - 使用SSL加密读取
   * @param[out] buffer 接收缓冲区
   * @param[in] length 缓冲区长度
   * @return 实际读取的字节数
   * @details 重写父类的read方法，通过SSL进行加密数据读取
   */
  virtual int read(void* buffer, size_t length) override;

  /**
   * @brief 重写写入方法 - 使用SSL加密写入
   * @param[in] buffer 发送缓冲区
   * @param[in] length 发送数据长度
   * @return 实际写入的字节数
   * @details 重写父类的write方法，通过SSL进行加密数据写入
   */
  virtual int write(const void* buffer, size_t length) override;

  /**
   * @brief 重写关闭方法 - 正确处理SSL连接关闭
   * @details 重写父类的close方法，先关闭SSL连接，再关闭底层socket
   */
  virtual void close() override;

  /**
   * @brief 执行SSL握手
   * @return bool 握手是否成功
   *         true表示握手成功，可以进行HTTP通信
   *         false表示握手失败或需要更多数据
   * @details 执行SSL/TLS握手协议，建立加密通信信道
   *          对于服务器端，执行server-side握手
   *          握手成功后才能进行HTTP请求接收和响应发送
   *          会自动处理握手过程中的各种状态和错误
   */
  bool doHandshake();

      /**
   * @brief 获取SSL上下文
   * @return SslContext::ptr SSL上下文智能指针
   * @details 返回当前会话使用的SSL上下文对象
   *          可用于获取SSL配置信息或进行高级SSL操作
   */
  SslContext::ptr getSslContext() const { return m_ctx; }

  /**
   * @brief 设置SSL上下文
   * @param[in] ctx SSL上下文智能指针
   * @details 设置会话使用的SSL上下文
   *          通常在创建会话后、握手前调用
   *          用于动态配置SSL参数
   */
  void setSslContext(SslContext::ptr ctx) { m_ctx = ctx; }

  /**
   * @brief 获取客户端证书
   * @return X509* 客户端证书指针，如果客户端未提供证书则返回nullptr
   * @details 获取客户端在SSL握手过程中提供的证书
   *          只有在启用客户端证书验证时才会有返回值
   *          返回的证书指针需要调用者手动释放
   */
  X509* getClientCertificate() const;

  /**
   * @brief 验证客户端证书
   * @return bool 验证是否成功
   *         true表示客户端证书验证通过
   *         false表示没有客户端证书或验证失败
   * @details 验证客户端证书的有效性，包括：
   *          - 证书是否在有效期内
   *          - 证书签名是否有效
   *          - 证书是否被信任的CA签发
   *          - 自定义验证逻辑（如果设置了验证回调）
   */
  bool verifyClientCertificate() const;

  /**
   * @brief 获取SSL连接信息
   * @return std::string SSL连接信息字符串
   * @details 返回SSL连接的详细信息，包括：
   *          - TLS版本
   *          - 密码套件
   *          - 证书信息
   *          - 连接状态等
   *          主要用于调试和日志记录
   */
  std::string getSSLInfo() const;

  /**
   * @brief 获取使用的密码套件
   * @return std::string 密码套件名称
   * @details 返回当前SSL连接协商使用的密码套件名称
   *          如"TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384"
   *          用于安全审计和性能分析
   */
  std::string getCipherSuite() const;

  /**
   * @brief 获取TLS版本
   * @return std::string TLS版本字符串
   * @details 返回当前SSL连接使用的TLS协议版本
   *          如"TLSv1.2"、"TLSv1.3"等
   *          用于兼容性检查和安全审计
   */
  std::string getTLSVersion() const;

  /**
   * @brief 获取SSL对象
   * @return SSL* SSL对象指针
   * @details 返回当前会话使用的SSL对象
   *          用于高级SSL操作和状态查询
   */
  SSL* getSSL() const { return m_ssl; }

  /**
   * @brief 检查SSL连接是否有效
   * @return bool 连接是否有效
   */
  bool isSSLValid() const;

private:
  /// SSL上下文 - 存储SSL配置信息，用于创建SSL对象和管理SSL参数
  SslContext::ptr m_ctx;
  /// SSL连接对象 - OpenSSL的SSL连接对象
  SSL* m_ssl;
  /// 是否拥有SSL对象所有权 - 控制析构时是否释放SSL对象
  bool m_owner_ssl;
  /// 是否已完成握手 - 标记SSL握手是否成功完成，控制HTTP通信的开始
  bool m_handshake_done;
};

}  // namespace https
}  // namespace sylar 