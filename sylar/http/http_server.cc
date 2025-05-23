/*
 * @Author: Nana5aki
 * @Date: 2025-04-26 22:25:17
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-05-01 19:01:06
 * @FilePath: /sylar_from_nanasaki/sylar/http/http_server.cc
 */
#include "http_server.h"
#include "sylar/log.h"
// #include "servlets/config_servlet.h"
// #include "servlets/status_servlet.h"

namespace sylar {
namespace http {

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

HttpServer::HttpServer(bool keepalive, sylar::IOManager* worker, sylar::IOManager* io_worker,
                       sylar::IOManager* accept_worker)
  : TcpServer(io_worker, accept_worker)
  , m_isKeepalive(keepalive) {
  m_dispatch.reset(new ServletDispatch);

  m_type = "http";
  // m_dispatch->addServlet("/_/status", Servlet::ptr(new StatusServlet));
  // m_dispatch->addServlet("/_/config", Servlet::ptr(new ConfigServlet));
}

void HttpServer::setName(const std::string& v) {
  TcpServer::setName(v);
  m_dispatch->setDefault(std::make_shared<NotFoundServlet>(v));
}

void HttpServer::handleClient(Socket::ptr client) {
  SYLAR_LOG_DEBUG(g_logger) << "handleClient " << *client;
  HttpSession::ptr session(new HttpSession(client));
  do {
    auto req = session->recvRequest();
    if (!req) {
      SYLAR_LOG_DEBUG(g_logger) << "recv http request fail, errno=" << errno
                                << " errstr=" << strerror(errno) << " cliet:" << *client
                                << " keep_alive=" << m_isKeepalive;
      break;
    }

    HttpResponse::ptr rsp(new HttpResponse(req->getVersion(), req->isClose() || !m_isKeepalive));
    rsp->setHeader("Server", getName());
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