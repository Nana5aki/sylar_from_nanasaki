/*
 * @Author: Nana5aki
 * @Date: 2025-01-02 10:00:00
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-07-13 00:22:47
 * @FilePath: /sylar_from_nanasaki/sylar/https/https_server.cc
 */
#include "https_server.h"
#include "https_session.h"
#include "sylar/log.h"
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/x509.h>

namespace sylar {
namespace https {

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

HttpsServer::HttpsServer(bool keepalive, sylar::IOManager* worker, sylar::IOManager* io_worker,
                         sylar::IOManager* accept_worker)
  : TcpServer(io_worker, accept_worker)
  , m_is_keepalive(keepalive)
  , m_ssl_initialized(false)
  , m_verify_mode(SSL_VERIFY_NONE)
  , m_verify_callback(nullptr)
  , m_password_callback(nullptr) {

  m_dispatch.reset(new sylar::http::ServletDispatch);
  m_type = "https";

  // 初始化SSL库
  initSSLLibrary();
}

HttpsServer::~HttpsServer() {
  cleanupSSLLibrary();
}

void HttpsServer::setName(const std::string& v) {
  TcpServer::setName(v);
  m_dispatch->setDefault(std::make_shared<sylar::http::NotFoundServlet>(v));
}

bool HttpsServer::loadCertificates(const std::string& cert_file, const std::string& key_file) {
  m_cert_file = cert_file;
  m_key_file = key_file;

  if (!m_ssl_ctx) {
    m_ssl_ctx = SslContext::Create();
  }

  return m_ssl_ctx->loadCertificates(cert_file, key_file);
}

void HttpsServer::setVerifyMode(int mode) {
  m_verify_mode = mode;
  if (m_ssl_ctx) {
    m_ssl_ctx->setVerifyMode(mode);
  }
}

void HttpsServer::setVerifyCallback(int (*callback)(int, X509_STORE_CTX*)) {
  m_verify_callback = callback;
  if (m_ssl_ctx) {
    m_ssl_ctx->setVerifyCallback(callback);
  } else {
    SYLAR_LOG_ERROR(g_logger) << "SSL context not initialized";
  }
}

void HttpsServer::setPasswordCallback(int (*callback)(char*, int, int, void*)) {
  m_password_callback = callback;
  if (m_ssl_ctx) {
    m_ssl_ctx->setPasswordCallback(callback);
  } else {
    SYLAR_LOG_ERROR(g_logger) << "SSL context not initialized";
  }
}

bool HttpsServer::initSSL() {
  if (m_ssl_initialized) {
    return true;
  }

  if (!m_ssl_ctx) {
    m_ssl_ctx = SslContext::Create();
  }

  // 如果有证书文件，加载它们
  if (!m_cert_file.empty() && !m_key_file.empty()) {
    if (!m_ssl_ctx->loadCertificates(m_cert_file, m_key_file)) {
      SYLAR_LOG_ERROR(g_logger) << "Failed to load certificates";
      return false;
    }
  }

  // 设置验证模式
  m_ssl_ctx->setVerifyMode(m_verify_mode);

  // 设置回调函数
  if (m_verify_callback) {
    m_ssl_ctx->setVerifyCallback(m_verify_callback);
  }

  if (m_password_callback) {
    m_ssl_ctx->setPasswordCallback(m_password_callback);
  }

  m_ssl_initialized = true;
  SYLAR_LOG_INFO(g_logger) << "HTTPS server SSL initialized successfully";
  return true;
}

bool HttpsServer::start() {
  if (!initSSL()) {
    SYLAR_LOG_ERROR(g_logger) << "Failed to initialize SSL";
    return false;
  }

  return TcpServer::start();
}

void HttpsServer::stop() {
  TcpServer::stop();
  // SSL清理由析构函数处理
}

std::string HttpsServer::toString(const std::string& prefix) {
  std::stringstream ss;
  ss << prefix << "[type=" << m_type << " name=" << m_name
     << " ssl=" << (m_ssl_initialized ? "enabled" : "disabled");
  if (m_ssl_initialized) {
    ss << " cert=" << m_cert_file << " key=" << m_key_file;
  }
  ss << "]" << std::endl;
  return ss.str();
}

void HttpsServer::handleClient(Socket::ptr client) {
  SYLAR_LOG_DEBUG(g_logger) << "handleClient " << *client;

  if (!m_ssl_initialized) {
    SYLAR_LOG_ERROR(g_logger) << "SSL not initialized";
    return;
  }

  // 创建HTTPS会话
  HttpsSession::ptr session = std::make_shared<HttpsSession>(client, m_ssl_ctx);

  // 执行SSL握手
  if (!session->doHandshake()) {
    SYLAR_LOG_ERROR(g_logger) << "SSL handshake failed for client " << *client;
    return;
  }

  SYLAR_LOG_DEBUG(g_logger) << "SSL handshake successful for client " << *client;

  // 处理HTTP请求
  do {
    auto req = session->recvRequest();
    if (!req) {
      SYLAR_LOG_DEBUG(g_logger) << "recv https request fail, errno=" << errno
                                << " errstr=" << strerror(errno) << " client:" << *client
                                << " keep_alive=" << m_is_keepalive;
      break;
    }

    // 创建HTTP响应
    sylar::http::HttpResponse::ptr rsp = std::make_shared<sylar::http::HttpResponse>(
      req->getVersion(), req->isClose() || !m_is_keepalive);
    rsp->setHeader("Server", getName());

    // 处理请求
    m_dispatch->handle(req, rsp, session);

    // 发送响应
    session->sendResponse(rsp);

    // 检查是否需要关闭连接
    if (!m_is_keepalive || req->isClose()) {
      break;
    }

  } while (true);

  session->close();
}

void HttpsServer::initSSLLibrary() {
  static bool ssl_lib_initialized = false;

  if (!ssl_lib_initialized) {
    // 初始化OpenSSL库
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    // 初始化随机数生成器
    if (RAND_poll() == 0) {
      SYLAR_LOG_WARN(g_logger) << "RAND_poll() failed";
    }

    ssl_lib_initialized = true;
    SYLAR_LOG_INFO(g_logger) << "OpenSSL library initialized";
  }
}

void HttpsServer::cleanupSSLLibrary() {
  // 可能有其他地方还在使用OpenSSL

  // ERR_free_strings();
  // EVP_cleanup();
  // CRYPTO_cleanup_all_ex_data();

  SYLAR_LOG_DEBUG(g_logger) << "OpenSSL library cleanup (skipped for thread safety)";
}

}   // namespace https
}   // namespace sylar