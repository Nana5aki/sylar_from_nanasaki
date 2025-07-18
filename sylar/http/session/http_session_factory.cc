/*
 * @Author: Nana5aki
 * @Date: 2025-07-13 16:05:00
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-07-18 23:01:15
 * @FilePath: /sylar_from_nanasaki/sylar/http/session/http_session_factory.cc
 */

#include "http_session_factory.h"
#include "../protocol/http11_protocol.h"
#include "../transport/ssl_transport.h"
#include "../transport/tcp_transport.h"
#include "http_session_impl.h"
#include "sylar/mutex.h"
#include "sylar/streams/ssl_socket_stream.h"

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

IHttpSession::ptr HttpSessionFactory::createHttpSession(Socket::ptr socket,
                                                        const HttpProtocolType& protocolVersion) {

  auto protocol = createProtocol(protocolVersion);
  return std::make_shared<HttpSessionImpl>(protocol, std::make_shared<TcpTransport>(socket));
}

IHttpSession::ptr HttpSessionFactory::createHttpsSession(Socket::ptr socket,
                                                         SslContext::ptr ssl_context,
                                                         const HttpProtocolType& protocolVersion) {
  auto protocol = createProtocol(protocolVersion);
  return std::make_shared<HttpSessionImpl>(protocol,
                                           std::make_shared<SslTransport>(socket, ssl_context));
}

IHttpProtocol::ptr HttpSessionFactory::createProtocol(const HttpProtocolType& protocolVersion) {
  if (protocolVersion == HttpProtocolType::HTTP1_1) {
    return std::make_shared<Http11Protocol>();
  }

  return nullptr;
}

}   // namespace http
}   // namespace sylar