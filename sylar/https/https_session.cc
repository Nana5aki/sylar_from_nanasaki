/*
 * @Author: Nana5aki
 * @Date: 2025-01-02 10:00:00
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-07-12 18:02:17
 * @FilePath: /sylar_from_nanasaki/sylar/https/https_session.cc
 * @Description: HTTPS会话实现文件 - 提供HTTPS服务器端会话处理功能
 *               基于SSL套接字流实现HTTP协议的加密传输
 */
#include "https_session.h"
#include "sylar/http/http_request_parser.h"
#include "sylar/log.h"
#include <openssl/x509.h>
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
  : SslSocketStream(sock, ssl, owner)
  , m_handshake_done(false) {
  // 初始化握手完成标志为false，需要后续调用doHandshake()完成握手
}

/**
 * @brief 构造函数实现 - 使用SSL上下文创建新连接
 * @details 基于SSL上下文创建新的SSL对象，适用于动态创建SSL连接的场景
 */
HttpsSession::HttpsSession(Socket::ptr sock, SslContext::ptr ctx, bool owner)
  : SslSocketStream(sock, nullptr, owner)
  , m_ctx(ctx)
  , m_handshake_done(false) {
  if (m_ctx) {
    // 从SSL上下文创建新的SSL对象
    m_ssl = m_ctx->createSSL();
    if (m_ssl) {
      // 将SSL对象与socket文件描述符关联
      SSL_set_fd(m_ssl, sock->getSocket());
    }
  }
}

sylar::http::HttpRequest::ptr HttpsSession::recvRequest() {
  // 如果还没有完成握手，先执行握手
  if (!m_handshake_done) {
    if (!doHandshake()) {
      SYLAR_LOG_ERROR(g_logger) << "SSL handshake failed";
      return nullptr;
    }
  }

  sylar::http::HttpRequestParser::ptr parser = std::make_shared<sylar::http::HttpRequestParser>();
  uint64_t buff_size = sylar::http::HttpRequestParser::GetHttpRequestBufferSize();
  std::shared_ptr<char> buffer(new char[buff_size], [](char* ptr) { delete[] ptr; });
  char* data = buffer.get();
  int offset = 0;

  do {
    int len = read(data + offset, buff_size - offset);
    if (len <= 0) {
      SYLAR_LOG_ERROR(g_logger) << "HTTPS recv request failed, len=" << len;
      close();
      return nullptr;
    }
    len += offset;
    size_t nparse = parser->execute(data, len);
    if (parser->hasError()) {
      SYLAR_LOG_ERROR(g_logger) << "HTTPS request parser error";
      close();
      return nullptr;
    }
    offset = len - nparse;
    if (offset == (int)buff_size) {
      SYLAR_LOG_ERROR(g_logger) << "HTTPS request buffer overflow";
      close();
      return nullptr;
    }
    if (parser->isFinished()) {
      break;
    }
  } while (true);

  return parser->getData();
}

int HttpsSession::sendResponse(sylar::http::HttpResponse::ptr rsp) {
  if (!m_handshake_done) {
    SYLAR_LOG_ERROR(g_logger) << "SSL handshake not completed";
    return -1;
  }

  std::stringstream ss;
  ss << *rsp;
  std::string data = ss.str();
  return writeFixSize(data.c_str(), data.size());
}

bool HttpsSession::doHandshake() {
  if (!m_ssl || !isConnected()) {
    SYLAR_LOG_ERROR(g_logger) << "Invalid SSL connection for handshake";
    return false;
  }

  if (m_handshake_done) {
    return true;
  }

  // 设置为服务端模式
  SSL_set_accept_state(m_ssl);

  bool ret = SslSocketStream::doHandshake();
  if (ret) {
    m_handshake_done = true;
  }
  return ret;
}

X509* HttpsSession::getClientCertificate() const {
  if (!m_ssl) {
    return nullptr;
  }
  return SSL_get_peer_certificate(m_ssl);
}

bool HttpsSession::verifyClientCertificate() const {
  X509* cert = getClientCertificate();
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

}   // namespace https
}   // namespace sylar