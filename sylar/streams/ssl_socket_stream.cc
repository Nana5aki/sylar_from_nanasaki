/*
 * @Author: Nana5aki
 * @Date: 2025-07-06 16:36:46
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-07-13 00:14:43
 * @FilePath: /sylar_from_nanasaki/sylar/streams/ssl_socket_stream.cc
 */

#include "ssl_socket_stream.h"
#include "sylar/bytearray.h"
#include "sylar/log.h"
#include <openssl/pem.h>
#include <openssl/x509.h>

namespace sylar {

// 系统日志器，用于记录SSL相关的调试和错误信息
static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

/**
 * @brief SslSocketStream构造函数实现
 */
SslSocketStream::SslSocketStream(Socket::ptr sock, SSL* ssl, bool owner)
  : SocketStream(sock, owner)
  , m_ssl(ssl)
  , m_owner_ssl(true) {
  // 如果SSL对象存在，将其与socket文件描述符关联
  // 这是SSL连接的必要步骤，让SSL对象知道要使用哪个socket进行通信
  if (m_ssl) {
    SSL_set_fd(m_ssl, sock->getSocket());
  }
}

/**
 * @brief SslSocketStream析构函数实现
 * @details 安全清理SSL连接和相关资源
 *          先执行SSL层的优雅关闭，然后释放SSL对象
 */
SslSocketStream::~SslSocketStream() {
  // 只有当拥有SSL对象所有权时才进行清理
  if (m_ssl && m_owner_ssl) {
    // 执行SSL握手的反向过程，发送close_notify消息
    // 通知对端即将关闭SSL连接，这是SSL协议的优雅关闭方式
    SSL_shutdown(m_ssl);
    // 释放SSL对象占用的内存和相关资源
    SSL_free(m_ssl);
  }
  // 父类析构函数会自动调用，处理底层socket的关闭
}

/**
 * @brief SSL读取数据实现
 * @details 从SSL连接读取加密数据并自动解密
 *          处理各种SSL错误情况，提供详细的错误日志
 */
int SslSocketStream::read(void* buffer, size_t length) {
  // 检查SSL对象和连接状态的有效性
  if (!m_ssl || !isConnected()) {
    return -1;
  }

  // 调用OpenSSL的SSL_read函数读取并解密数据
  // SSL_read会自动处理SSL记录层的解包和应用数据的解密
  int ret = SSL_read(m_ssl, buffer, length);

  // 如果读取失败或连接关闭，需要详细分析错误原因
  if (ret <= 0) {
    // 获取SSL特定的错误码，用于判断错误类型
    int error = SSL_get_error(m_ssl, ret);
    switch (error) {
    case SSL_ERROR_NONE:
      // 实际上不应该进入这个分支，因为ret <= 0
      return ret;
    case SSL_ERROR_ZERO_RETURN:
      // 对端正常关闭了SSL连接，发送了close_notify消息
      SYLAR_LOG_DEBUG(g_logger) << "SSL connection closed by peer";
      return 0;
    case SSL_ERROR_WANT_READ:
    case SSL_ERROR_WANT_WRITE:
      // SSL层需要更多数据才能完成读取操作
      // 对于非阻塞socket，这是正常情况，需要稍后重试
      SYLAR_LOG_DEBUG(g_logger) << "SSL_read wants more data";
      return -1;
    case SSL_ERROR_SYSCALL:
      // 底层socket系统调用出错
      SYLAR_LOG_ERROR(g_logger) << "SSL_read syscall error: " << strerror(errno);
      return -1;
    default:
      // 其他SSL协议错误，如证书验证失败、协议错误等
      SYLAR_LOG_ERROR(g_logger) << "SSL_read error: " << error;
      return -1;
    }
  }
  // 成功读取数据，返回实际读取的字节数
  return ret;
}

/**
 * @brief SSL读取数据到ByteArray实现
 * @details 先读取到临时缓冲区，然后写入ByteArray对象
 *          这是read(void*, size_t)方法的便利封装
 */
int SslSocketStream::read(ByteArray::ptr ba, size_t length) {
  // 检查SSL对象和连接状态的有效性
  if (!m_ssl || !isConnected()) {
    return -1;
  }

  // 创建临时缓冲区来接收解密后的数据
  std::vector<char> buffer(length);
  // 调用基础的read方法进行实际的SSL数据读取
  int ret = read(buffer.data(), length);

  // 如果成功读取到数据，将其写入ByteArray对象
  if (ret > 0) {
    ba->write(buffer.data(), ret);
  }
  // 返回实际读取的字节数（可能小于请求的长度）
  return ret;
}

/**
 * @brief SSL写入数据实现
 * @details 将明文数据加密后通过SSL连接发送
 *          处理各种SSL写入错误情况，提供详细的错误日志
 */
int SslSocketStream::write(const void* buffer, size_t length) {
  // 检查SSL对象和连接状态的有效性
  if (!m_ssl || !isConnected()) {
    return -1;
  }

  // 调用OpenSSL的SSL_write函数加密并发送数据
  // SSL_write会自动处理应用数据的加密和SSL记录层的封装
  int ret = SSL_write(m_ssl, buffer, length);

  // 如果写入失败或连接关闭，需要详细分析错误原因
  if (ret <= 0) {
    // 获取SSL特定的错误码，用于判断错误类型
    int error = SSL_get_error(m_ssl, ret);
    switch (error) {
    case SSL_ERROR_NONE:
      // 实际上不应该进入这个分支，因为ret <= 0
      return ret;
    case SSL_ERROR_ZERO_RETURN:
      // 对端正常关闭了SSL连接，不再接收数据
      SYLAR_LOG_DEBUG(g_logger) << "SSL connection closed by peer";
      return 0;
    case SSL_ERROR_WANT_READ:
    case SSL_ERROR_WANT_WRITE:
      // SSL层需要更多缓冲区空间或网络可写状态才能完成写入
      // 对于非阻塞socket，这是正常情况，需要稍后重试
      SYLAR_LOG_DEBUG(g_logger) << "SSL_write wants more data";
      return -1;
    case SSL_ERROR_SYSCALL:
      // 底层socket系统调用出错，可能是网络断开
      SYLAR_LOG_ERROR(g_logger) << "SSL_write syscall error: " << strerror(errno);
      return -1;
    default:
      // 其他SSL协议错误，如重新协商失败等
      SYLAR_LOG_ERROR(g_logger) << "SSL_write error: " << error;
      return -1;
    }
  }
  // 成功写入数据，返回实际写入的字节数
  return ret;
}

/**
 * @brief 从ByteArray写入SSL数据实现
 * @details 先从ByteArray读取数据到临时缓冲区，然后通过SSL发送
 */
int SslSocketStream::write(ByteArray::ptr ba, size_t length) {
  // 检查SSL对象和连接状态的有效性
  if (!m_ssl || !isConnected()) {
    return -1;
  }

  // 创建临时缓冲区来存储从ByteArray读取的数据
  std::vector<char> buffer(length);
  // 从ByteArray对象读取指定长度的数据
  ba->read(buffer.data(), length);
  // 调用基础的write方法进行实际的SSL数据发送
  return write(buffer.data(), length);
}

/**
 * @brief SSL连接关闭实现
 * @details 执行SSL层的优雅关闭，然后关闭底层socket
 */
void SslSocketStream::close() {
  // 如果SSL对象存在，先执行SSL层的关闭
  if (m_ssl) {
    // SSL_shutdown发送close_notify消息，通知对端关闭连接
    // 这是SSL协议要求的优雅关闭方式，确保数据完整性
    SSL_shutdown(m_ssl);
  }
  // 调用父类方法关闭底层socket连接
  SocketStream::close();
}

/**
 * @brief SSL握手实现
 * @details 执行SSL/TLS握手协议，建立加密通信信道
 *          握手过程包括协议版本协商、密码套件选择、证书交换等步骤
 */
bool SslSocketStream::doHandshake() {
  // 检查SSL对象和连接状态的基本有效性
  if (!m_ssl || !isConnected()) {
    return false;
  }

  // 执行SSL握手，这是一个可能需要多轮交互的过程
  // SSL_do_handshake会根据当前SSL对象的状态（客户端或服务端）
  // 自动执行相应的握手步骤
  int ret = SSL_do_handshake(m_ssl);

  // 返回值为1表示握手成功完成
  if (ret == 1) {
    SYLAR_LOG_DEBUG(g_logger) << "SSL handshake successful";
    return true;
  }

  // 握手未完成或出错，分析具体错误原因
  int error = SSL_get_error(m_ssl, ret);
  switch (error) {
  case SSL_ERROR_WANT_READ:
  case SSL_ERROR_WANT_WRITE:
    // 握手过程需要更多数据或等待网络可写
    // 对于非阻塞socket，这是正常情况，需要稍后重试
    SYLAR_LOG_DEBUG(g_logger) << "SSL handshake wants more data";
    return false;
  case SSL_ERROR_ZERO_RETURN:
    // 对端在握手过程中关闭了连接
    SYLAR_LOG_DEBUG(g_logger) << "SSL handshake: connection closed";
    return false;
  case SSL_ERROR_SYSCALL:
    // 底层系统调用错误，通常是网络问题
    SYLAR_LOG_ERROR(g_logger) << "SSL handshake syscall error: " << strerror(errno);
    return false;
  default:
    // 其他SSL协议错误，如证书验证失败、协议不匹配等
    SYLAR_LOG_ERROR(g_logger) << "SSL handshake error: " << error;
    return false;
  }
}

/**
 * @brief 检查SSL连接有效性实现
 * @details 验证SSL对象是否存在且底层socket连接正常
 */
bool SslSocketStream::isValid() const {
  // 检查SSL对象是否存在且底层socket仍然连接
  // 注意：这里不检查SSL握手是否完成，只检查基础连接状态
  return m_ssl != nullptr && isConnected();
}

// ==================== SslContext类实现 ====================

/**
 * @brief SslContext构造函数实现
 * @details 创建并初始化SSL上下文，设置基本的安全选项
 */
SslContext::SslContext(const SSL_METHOD* method) {
  // 如果未指定SSL方法，使用默认的TLS_method
  // TLS_method支持所有TLS版本，让客户端和服务端自动协商
  if (!method) {
    method = TLS_method();
  }

  // 创建SSL上下文对象，这是所有SSL连接的配置基础
  m_ctx = SSL_CTX_new(method);
  if (!m_ctx) {
    SYLAR_LOG_ERROR(g_logger) << "SSL_CTX_new failed";
    throw std::runtime_error("SSL_CTX_new failed");
  }

  // 设置SSL安全选项，禁用不安全的旧版本协议
  // SSL_OP_NO_SSLv2: 禁用SSLv2协议（已知存在严重安全漏洞）
  // SSL_OP_NO_SSLv3: 禁用SSLv3协议（POODLE攻击漏洞）
  SSL_CTX_set_options(m_ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);

  // 设置SSL模式，启用自动重试机制
  // SSL_MODE_AUTO_RETRY: 当SSL操作因为重新协商而失败时自动重试
  SSL_CTX_set_mode(m_ctx, SSL_MODE_AUTO_RETRY);
}

/**
 * @brief SslContext析构函数实现
 * @details 释放SSL上下文占用的资源
 */
SslContext::~SslContext() {
  if (m_ctx) {
    // 释放SSL_CTX对象及其关联的所有资源
    // 包括证书、私钥、CA证书列表等
    SSL_CTX_free(m_ctx);
  }
}

/**
 * @brief 创建SslContext智能指针的工厂方法实现
 * @details 推荐使用这个静态方法创建SslContext对象
 */
SslContext::ptr SslContext::Create(const SSL_METHOD* method) {
  return std::make_shared<SslContext>(method);
}

/**
 * @brief 证书和私钥加载实现
 * @details 加载PEM格式的证书和私钥文件，并验证其匹配性
 *          这是SSL服务器配置的关键步骤
 */
bool SslContext::loadCertificates(const std::string& cert_file, const std::string& key_file) {
  // 检查SSL上下文是否有效
  if (!m_ctx) {
    return false;
  }

  // 加载证书文件（PEM格式）
  // SSL_CTX_use_certificate_file会解析证书文件并加载到SSL上下文中
  if (SSL_CTX_use_certificate_file(m_ctx, cert_file.c_str(), SSL_FILETYPE_PEM) <= 0) {
    SYLAR_LOG_ERROR(g_logger) << "Failed to load certificate file: " << cert_file;
    return false;
  }

  // 加载私钥文件（PEM格式）
  // SSL_CTX_use_PrivateKey_file会解析私钥文件并加载到SSL上下文中
  if (SSL_CTX_use_PrivateKey_file(m_ctx, key_file.c_str(), SSL_FILETYPE_PEM) <= 0) {
    SYLAR_LOG_ERROR(g_logger) << "Failed to load private key file: " << key_file;
    return false;
  }

  // 验证私钥与证书是否匹配
  // 这是安全的重要检查，确保私钥可以解密使用此证书加密的数据
  if (!SSL_CTX_check_private_key(m_ctx)) {
    SYLAR_LOG_ERROR(g_logger) << "Private key does not match certificate";
    return false;
  }

  SYLAR_LOG_INFO(g_logger) << "Certificate and private key loaded successfully";
  return true;
}

/**
 * @brief 设置SSL证书验证模式实现
 * @details 配置SSL连接的证书验证策略
 */
void SslContext::setVerifyMode(int mode) {
  if (m_ctx) {
    // 设置证书验证模式，不使用自定义验证回调
    // mode参数可以是SSL_VERIFY_NONE、SSL_VERIFY_PEER等
    SSL_CTX_set_verify(m_ctx, mode, nullptr);
  }
}

/**
 * @brief 设置证书验证回调函数实现
 * @details 设置自定义的证书验证逻辑，允许实现额外的验证检查
 */
void SslContext::setVerifyCallback(int (*callback)(int, X509_STORE_CTX*)) {
  if (m_ctx) {
    // 保持当前的验证模式，只替换验证回调函数
    // 回调函数可以实现自定义的证书验证逻辑
    SSL_CTX_set_verify(m_ctx, SSL_CTX_get_verify_mode(m_ctx), callback);
  }
}

/**
 * @brief 设置私钥密码回调函数实现
 * @details 当私钥文件受密码保护时，通过回调函数提供解密密码
 */
void SslContext::setPasswordCallback(pem_password_cb callback) {
  if (m_ctx) {
    // 设置默认的密码回调函数
    // 当加载受密码保护的私钥时，OpenSSL会调用此回调获取密码
    SSL_CTX_set_default_passwd_cb(m_ctx, callback);
  }
}

/**
 * @brief 创建新的SSL连接对象实现
 * @details 基于当前SSL上下文创建新的SSL连接实例
 */
SSL* SslContext::createSSL() {
  // 检查SSL上下文是否有效
  if (!m_ctx) {
    return nullptr;
  }

  // 创建新的SSL连接对象
  // SSL对象继承SSL_CTX的所有配置（证书、验证模式、密码套件等）
  SSL* ssl = SSL_new(m_ctx);
  if (!ssl) {
    SYLAR_LOG_ERROR(g_logger) << "SSL_new failed";
    return nullptr;
  }

  return ssl;
}

}   // namespace sylar