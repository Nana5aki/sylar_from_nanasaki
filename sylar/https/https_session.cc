/*
 * @Author: Nana5aki
 * @Date: 2025-01-02 10:00:00
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-07-12 18:02:17
 * @FilePath: /sylar_from_nanasaki/sylar/https/https_session.cc
 * @Description: HTTPS会话实现文件 - 提供HTTPS服务器端会话处理功能
 *               基于HttpSession实现SSL/TLS加密的HTTP协议传输
 */
#include "https_session.h"
#include "sylar/log.h"
#include <openssl/x509.h>
#include <openssl/err.h>
#include <sstream>

namespace sylar {
namespace https {

// 系统日志器，用于记录HTTPS会话相关的调试和错误信息
static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

/**
 * @brief 构造函数实现 - 使用已存在的SSL对象
 * @details 适用于已经创建好SSL对象的场景，通常用于服务器接受连接
 */
HttpsSession::HttpsSession(Socket::ptr sock, SSL* ssl, bool owner)
  : sylar::http::HttpSession(sock, owner)
  , m_ssl(ssl)
  , m_owner_ssl(true)
  , m_handshake_done(false) {
  // 如果SSL对象存在，将其与socket关联
  if (m_ssl && sock) {
    SSL_set_fd(m_ssl, sock->getSocket());
  }
}

/**
 * @brief 构造函数实现 - 使用SSL上下文创建新连接
 * @details 基于SSL上下文创建新的SSL对象，适用于动态创建SSL连接的场景
 */
HttpsSession::HttpsSession(Socket::ptr sock, SslContext::ptr ctx, bool owner)
  : sylar::http::HttpSession(sock, owner)
  , m_ctx(ctx)
  , m_ssl(nullptr)
  , m_owner_ssl(true)
  , m_handshake_done(false) {
  if (m_ctx) {
    // 从SSL上下文创建新的SSL对象
    m_ssl = m_ctx->createSSL();
    if (m_ssl && sock) {
      // 将SSL对象与socket文件描述符关联
      SSL_set_fd(m_ssl, sock->getSocket());
    }
  }
}

/**
 * @brief 析构函数实现
 * @details 清理SSL对象和相关资源
 */
HttpsSession::~HttpsSession() {
  if (m_ssl && m_owner_ssl) {
    // 执行SSL优雅关闭
    SSL_shutdown(m_ssl);
    SSL_free(m_ssl);
    m_ssl = nullptr;
  }
}

int HttpsSession::read(void* buffer, size_t length) {
  if (!m_ssl) {
    SYLAR_LOG_ERROR(g_logger) << "SSL not initialized";
    return -1;
  }

  // 如果还没有完成握手，先执行握手
  if (!m_handshake_done) {
    if (!doHandshake()) {
      SYLAR_LOG_ERROR(g_logger) << "SSL handshake failed during read";
      return -1;
    }
  }

  int ret = SSL_read(m_ssl, buffer, length);
  if (ret <= 0) {
    int ssl_error = SSL_get_error(m_ssl, ret);
    switch (ssl_error) {
      case SSL_ERROR_WANT_READ:
      case SSL_ERROR_WANT_WRITE:
        // 需要更多数据，稍后重试
        errno = EAGAIN;
        return -1;
      case SSL_ERROR_ZERO_RETURN:
        // SSL连接被优雅关闭
        return 0;
      case SSL_ERROR_SYSCALL:
        // 系统调用错误
        if (errno == 0) {
          // 对端关闭连接
          return 0;
        }
        SYLAR_LOG_ERROR(g_logger) << "SSL_read syscall error: " << strerror(errno);
        return -1;
      case SSL_ERROR_SSL:
        // SSL协议错误
        SYLAR_LOG_ERROR(g_logger) << "SSL_read protocol error: " << ERR_error_string(ERR_get_error(), nullptr);
        return -1;
      default:
        SYLAR_LOG_ERROR(g_logger) << "SSL_read unknown error: " << ssl_error;
        return -1;
    }
  }

  return ret;
}

int HttpsSession::write(const void* buffer, size_t length) {
  if (!m_ssl) {
    SYLAR_LOG_ERROR(g_logger) << "SSL not initialized";
    return -1;
  }

  // 如果还没有完成握手，先执行握手
  if (!m_handshake_done) {
    if (!doHandshake()) {
      SYLAR_LOG_ERROR(g_logger) << "SSL handshake failed during write";
      return -1;
    }
  }

  int ret = SSL_write(m_ssl, buffer, length);
  if (ret <= 0) {
    int ssl_error = SSL_get_error(m_ssl, ret);
    switch (ssl_error) {
      case SSL_ERROR_WANT_READ:
      case SSL_ERROR_WANT_WRITE:
        // 需要更多数据，稍后重试
        errno = EAGAIN;
        return -1;
      case SSL_ERROR_ZERO_RETURN:
        // SSL连接被优雅关闭
        return 0;
      case SSL_ERROR_SYSCALL:
        // 系统调用错误
        if (errno == 0) {
          // 对端关闭连接
          return 0;
        }
        SYLAR_LOG_ERROR(g_logger) << "SSL_write syscall error: " << strerror(errno);
        return -1;
      case SSL_ERROR_SSL:
        // SSL协议错误
        SYLAR_LOG_ERROR(g_logger) << "SSL_write protocol error: " << ERR_error_string(ERR_get_error(), nullptr);
        return -1;
      default:
        SYLAR_LOG_ERROR(g_logger) << "SSL_write unknown error: " << ssl_error;
    return -1;
  }
  }

  return ret;
}

void HttpsSession::close() {
  if (m_ssl && m_owner_ssl) {
    // 只有拥有SSL对象所有权时才执行SSL层的关闭
    SSL_shutdown(m_ssl);
  }
  
  // 调用父类方法关闭底层socket连接
  sylar::http::HttpSession::close();
}

bool HttpsSession::doHandshake() {
  if (!m_ssl || !getSocket() || !getSocket()->isConnected()) {
    SYLAR_LOG_ERROR(g_logger) << "Invalid SSL connection for handshake";
    return false;
  }

  if (m_handshake_done) {
    return true;
  }

  // 设置为服务端模式
  SSL_set_accept_state(m_ssl);

  int ret = SSL_accept(m_ssl);
  if (ret == 1) {
    // 握手成功
    m_handshake_done = true;
    SYLAR_LOG_DEBUG(g_logger) << "SSL handshake completed successfully";
    return true;
  }

  int ssl_error = SSL_get_error(m_ssl, ret);
  switch (ssl_error) {
    case SSL_ERROR_WANT_READ:
    case SSL_ERROR_WANT_WRITE:
      // 需要更多数据，稍后重试
      SYLAR_LOG_DEBUG(g_logger) << "SSL handshake wants more data";
      errno = EAGAIN;
      return false;
    case SSL_ERROR_SYSCALL:
      SYLAR_LOG_ERROR(g_logger) << "SSL handshake syscall error: " << strerror(errno);
      return false;
    case SSL_ERROR_SSL:
      SYLAR_LOG_ERROR(g_logger) << "SSL handshake protocol error: " << ERR_error_string(ERR_get_error(), nullptr);
      return false;
    default:
      SYLAR_LOG_ERROR(g_logger) << "SSL handshake unknown error: " << ssl_error;
      return false;
  }
}

X509* HttpsSession::getClientCertificate() const {
  if (!m_ssl) {
    return nullptr;
  }
  return SSL_get_peer_certificate(m_ssl);
}

bool HttpsSession::verifyClientCertificate() const {
  if (!m_ssl) {
    return false;
  }

  X509* cert = SSL_get_peer_certificate(m_ssl);
  if (!cert) {
    return false;   // 没有客户端证书
  }

  long result = SSL_get_verify_result(m_ssl);
  X509_free(cert);

  return result == X509_V_OK;
}

std::string HttpsSession::getSSLInfo() const {
  if (!m_ssl) {
    return "No SSL connection";
  }

  std::stringstream ss;
  ss << "SSL Version: " << SSL_get_version(m_ssl) << "\n";
  ss << "Cipher Suite: " << SSL_get_cipher_name(m_ssl) << "\n";
  ss << "Cipher Bits: " << SSL_get_cipher_bits(m_ssl, nullptr) << "\n";

  X509* cert = getClientCertificate();
  if (cert) {
    char* subject = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
    char* issuer = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
    ss << "Client Certificate Subject: " << (subject ? subject : "N/A") << "\n";
    ss << "Client Certificate Issuer: " << (issuer ? issuer : "N/A") << "\n";

    if (subject) OPENSSL_free(subject);
    if (issuer) OPENSSL_free(issuer);
    X509_free(cert);
  } else {
    ss << "No client certificate\n";
  }

  return ss.str();
}

std::string HttpsSession::getCipherSuite() const {
  if (!m_ssl) {
    return "No SSL connection";
  }

  const char* cipher = SSL_get_cipher_name(m_ssl);
  return cipher ? cipher : "Unknown";
}

std::string HttpsSession::getTLSVersion() const {
  if (!m_ssl) {
    return "No SSL connection";
  }

  const char* version = SSL_get_version(m_ssl);
  return version ? version : "Unknown";
}

bool HttpsSession::isSSLValid() const {
  return m_ssl != nullptr && m_handshake_done;
}

}   // namespace https
}   // namespace sylar