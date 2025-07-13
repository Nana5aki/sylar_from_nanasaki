/*
 * @Author: Nana5aki
 * @Date: 2025-07-13 16:05:00
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-07-13 15:16:18
 * @FilePath: /sylar_from_nanasaki/sylar/http/session/http_session_factory.cc
 */

#include "http_session_factory.h"
#include "http_session_impl.h"
#include "../protocol/http11_protocol.h"
#include "../transport/tcp_transport.h"
#include "sylar/mutex.h"

namespace sylar {
namespace http {

HttpSessionFactory::ptr HttpSessionFactory::s_instance = nullptr;

HttpSessionFactory::ptr HttpSessionFactory::getInstance() {
  static Mutex s_mutex;
  if (!s_instance) {
    Mutex::Lock lock(s_mutex);
    if (!s_instance) {
      s_instance = std::shared_ptr<HttpSessionFactory>(new HttpSessionFactory());
    }
  }
  return s_instance;
}

IHttpSession::ptr HttpSessionFactory::createSession(Socket::ptr socket,
                                                   const std::string& protocolVersion,
                                                   const std::string& transportType) {
  auto transport = createTransport(socket, transportType);
  if (!transport) {
    return nullptr;
  }
  
  auto protocol = createProtocol(protocolVersion);
  if (!protocol) {
    return nullptr;
  }
  
  return createSession(protocol, transport);
}

IHttpSession::ptr HttpSessionFactory::createSession(IHttpTransport::ptr transport,
                                                   const std::string& protocolVersion) {
  auto protocol = createProtocol(protocolVersion);
  if (!protocol) {
    return nullptr;
  }
  
  return createSession(protocol, transport);
}

IHttpSession::ptr HttpSessionFactory::createSession(IHttpProtocol::ptr protocol,
                                                   IHttpTransport::ptr transport) {
  if (!protocol || !transport) {
    return nullptr;
  }
  
  return std::make_shared<HttpSessionImpl>(protocol, transport);
}

IHttpProtocol::ptr HttpSessionFactory::createProtocol(const std::string& version) {
  if (version == "1.1") {
    return std::make_shared<Http11Protocol>();
  }
  
  // TODO: 添加其他协议版本支持
  return nullptr;
}

IHttpTransport::ptr HttpSessionFactory::createTransport(Socket::ptr socket, const std::string& type) {
  if (type == "tcp") {
    return std::make_shared<TcpTransport>(socket);
  }
  
  // TODO: 添加其他传输类型支持（SSL等）
  return nullptr;
}

}  // namespace http
}  // namespace sylar 