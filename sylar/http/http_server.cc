/*
 * @Author: Nana5aki
 * @Date: 2025-04-26 22:25:17
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-07-18 22:41:19
 * @FilePath: /sylar_from_nanasaki/sylar/http/http_server.cc
 */
#include "http_server.h"
#include "session/http_session_factory.h"
#include "sylar/log.h"

namespace sylar {
namespace http {

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

HttpServer::HttpServer(bool keepalive, sylar::IOManager* worker, sylar::IOManager* io_worker,
                       sylar::IOManager* accept_worker)
  : TcpServer(io_worker, accept_worker)
  , m_isKeepalive(keepalive) {
  m_dispatch.reset(new ServletDispatch);
  m_type = "http";
}

void HttpServer::setName(const std::string& v) {
  TcpServer::setName(v);
  m_dispatch->setDefault(std::make_shared<NotFoundServlet>(v));
}

void HttpServer::handleClient(Socket::ptr client) {
  SYLAR_LOG_DEBUG(g_logger) << "handleClient " << *client;
  
  // 创建新架构的HTTP会话
  auto session = HttpSessionFactory::getInstance()->createHttpSession(client);
  
  do {
    auto req = session->recvRequest();
    if (!req) {
      SYLAR_LOG_DEBUG(g_logger) << "recv http request fail, errno=" << errno
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

}  // namespace http
}  // namespace sylar