/*
 * @Author: Nana5aki
 * @Date: 2025-07-18 00:51:57
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-07-19 17:33:17
 * @FilePath: /sylar_from_nanasaki/sylar/http/https_server.cc
 */

#include "https_server.h"
#include "session/http_session_factory.h"
#include "sylar/log.h"

namespace sylar {
namespace http {

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

HttpsServer::HttpsServer(const std::string& cert_file, const std::string& key_file, bool keepalive,
                         sylar::IOManager* worker, sylar::IOManager* io_worker,
                         sylar::IOManager* accept_worker)
  : TcpServer(io_worker, accept_worker)
  , m_isKeepalive(keepalive) {
  m_dispatch.reset(new ServletDispatch);
  m_type = "https";
  setSSLContext(cert_file, key_file);
}

void HttpsServer::setName(const std::string& v) {
  TcpServer::setName(v);
  m_dispatch->setDefault(std::make_shared<NotFoundServlet>(v));
}

bool HttpsServer::setSSLContext(const std::string& cert_file, const std::string& key_file) {
  // 创建SSL上下文
  m_ssl_context = SslContext::Create();
  if (!m_ssl_context) {
    SYLAR_LOG_ERROR(g_logger) << "Failed to create SSL context";
    return false;
  }

  // 加载证书和私钥
  if (!m_ssl_context->loadCertificates(cert_file, key_file)) {
    SYLAR_LOG_ERROR(g_logger) << "Failed to load SSL certificates: cert=" << cert_file
                              << " key=" << key_file;
    return false;
  }

  SYLAR_LOG_INFO(g_logger) << "SSL context initialized successfully: cert=" << cert_file
                           << " key=" << key_file;
  return true;
}

void HttpsServer::handleClient(Socket::ptr client) {
  SYLAR_LOG_DEBUG(g_logger) << "handleClient " << *client;

  if (!m_ssl_context) {
    SYLAR_LOG_ERROR(g_logger) << "SSL context not initialized, closing connection";
    client->close();
    return;
  }

  // 创建HTTP会话
  auto session = HttpSessionFactory::getInstance()->createHttpsSession(client, m_ssl_context);
  if (!session) {
    SYLAR_LOG_ERROR(g_logger) << "Failed to create HTTP session";
    client->close();
    return;
  }

  do {
    auto req = session->recvRequest();
    if (!req) {
      SYLAR_LOG_DEBUG(g_logger) << "recv https request fail, errno=" << errno
                                << " errstr=" << strerror(errno) << " client:" << *client
                                << " keep_alive=" << m_isKeepalive;
      break;
    }

    HttpResponse::ptr rsp(new HttpResponse(req->getVersion(), req->isClose() || !m_isKeepalive));
    rsp->setHeader("Server", getName());

    // 使用新的HttpSession接口
    m_dispatch->handle(req, rsp, session);
    session->sendResponse(rsp);

    if (!m_isKeepalive || req->isClose()) {
      break;
    }
  } while (true);

  session->close();
}

}   // namespace http
}   // namespace sylar
