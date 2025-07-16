/*
 * @Author: Nana5aki
 * @Date: 2025-07-16 01:13:06
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-07-16 23:52:10
 * @FilePath: /sylar_from_nanasaki/sylar/http/transport/ssl_transport.cc
 */

#include "ssl_transport.h"
#include "sylar/log.h"
#include <openssl/x509.h>
#include <openssl/err.h>
#include <sstream>

namespace sylar {
namespace http {

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

SslTransport::SslTransport(SslSocketStream::ptr ssl_stream)
  : m_ssl_stream(ssl_stream), m_handshake_done(false) {}

SslTransport::SslTransport(Socket::ptr socket, SslContext::ptr ssl_context, bool owner_socket)
  : m_ssl_stream(std::make_shared<SslSocketStream>(socket, ssl_context->createSSL(), owner_socket)), 
    m_handshake_done(false) {}

int SslTransport::read(void* buffer, size_t length) {
  if (!m_ssl_stream || !m_ssl_stream->isValid()) {
    SYLAR_LOG_ERROR(g_logger) << "SSL stream not valid";
    return -1;
  }

  // 如果还没有完成握手，先执行握手
  if (!m_handshake_done) {
    if (!doHandshake()) {
      SYLAR_LOG_ERROR(g_logger) << "SSL handshake failed during read";
      return -1;
    }
  }

  return m_ssl_stream->read(buffer, length);
}

int SslTransport::write(const void* buffer, size_t length) {
  if (!m_ssl_stream || !m_ssl_stream->isValid()) {
    SYLAR_LOG_ERROR(g_logger) << "SSL stream not valid";
    return -1;
  }

  // 如果还没有完成握手，先执行握手
  if (!m_handshake_done) {
    if (!doHandshake()) {
      SYLAR_LOG_ERROR(g_logger) << "SSL handshake failed during write";
      return -1;
    }
  }

  return m_ssl_stream->write(buffer, length);
}

int SslTransport::writeFixSize(const void* buffer, size_t length) {
  if (!m_ssl_stream || !m_ssl_stream->isValid()) {
    SYLAR_LOG_ERROR(g_logger) << "SSL stream not valid";
    return -1;
  }

  // 如果还没有完成握手，先执行握手
  if (!m_handshake_done) {
    if (!doHandshake()) {
      SYLAR_LOG_ERROR(g_logger) << "SSL handshake failed during writeFixSize";
      return -1;
    }
  }

  size_t written = 0;
  while (written < length) {
    int ret = m_ssl_stream->write(static_cast<const char*>(buffer) + written, 
                                  length - written);
    if (ret <= 0) {
      return ret;
    }
    written += ret;
  }
  return written;
}

void SslTransport::close() {
  if (m_ssl_stream) {
    m_ssl_stream->close();
  }
  m_handshake_done = false;
}

bool SslTransport::isConnected() {
  return m_ssl_stream && m_ssl_stream->isConnected();
}

std::string SslTransport::getLocalAddress() {
  if (!m_ssl_stream) {
    return "";
  }
  return m_ssl_stream->getLocalAddressString();
}

std::string SslTransport::getRemoteAddress() {
  if (!m_ssl_stream) {
    return "";
  }
  return m_ssl_stream->getRemoteAddressString();
}

Socket::ptr SslTransport::getSocket() {
  if (!m_ssl_stream) {
    return nullptr;
  }
  return m_ssl_stream->getSocket();
}

bool SslTransport::doHandshake() {
  if (!m_ssl_stream || !m_ssl_stream->isValid()) {
    SYLAR_LOG_ERROR(g_logger) << "Invalid SSL stream for handshake";
    return false;
  }

  if (m_handshake_done) {
    return true;
  }

  bool result = m_ssl_stream->doHandshake();
  if (result) {
    m_handshake_done = true;
    SYLAR_LOG_DEBUG(g_logger) << "SSL handshake completed successfully";
  } else {
    SYLAR_LOG_DEBUG(g_logger) << "SSL handshake failed or needs more data";
  }

  return result;
}

bool SslTransport::isSSLValid() const {
  return m_ssl_stream && m_ssl_stream->isValid() && m_handshake_done;
}

std::string SslTransport::getSSLInfo() const {
  if (!m_ssl_stream || !m_ssl_stream->getSSL()) {
    return "No SSL connection";
  }

  SSL* ssl = m_ssl_stream->getSSL();
  std::stringstream ss;
  ss << "SSL Version: " << SSL_get_version(ssl) << "\n";
  ss << "Cipher Suite: " << SSL_get_cipher_name(ssl) << "\n";
  ss << "Cipher Bits: " << SSL_get_cipher_bits(ssl, nullptr) << "\n";

  X509* cert = SSL_get_peer_certificate(ssl);
  if (cert) {
    char* subject = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
    char* issuer = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
    ss << "Peer Certificate Subject: " << (subject ? subject : "N/A") << "\n";
    ss << "Peer Certificate Issuer: " << (issuer ? issuer : "N/A") << "\n";

    if (subject) OPENSSL_free(subject);
    if (issuer) OPENSSL_free(issuer);
    X509_free(cert);
  } else {
    ss << "No peer certificate\n";
  }

  return ss.str();
}

std::string SslTransport::getCipherSuite() const {
  if (!m_ssl_stream || !m_ssl_stream->getSSL()) {
    return "No SSL connection";
  }

  SSL* ssl = m_ssl_stream->getSSL();
  const char* cipher = SSL_get_cipher_name(ssl);
  return cipher ? cipher : "Unknown";
}

std::string SslTransport::getTLSVersion() const {
  if (!m_ssl_stream || !m_ssl_stream->getSSL()) {
    return "No SSL connection";
  }

  SSL* ssl = m_ssl_stream->getSSL();
  const char* version = SSL_get_version(ssl);
  return version ? version : "Unknown";
}

SslSocketStream::ptr SslTransport::getSslSocketStream() const {
  return m_ssl_stream;
}

}   // namespace http
}   // namespace sylar