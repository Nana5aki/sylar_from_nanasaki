/*
 * @Author: Nana5aki
 * @Date: 2025-07-13 15:40:00
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-07-13 15:13:13
 * @FilePath: /sylar_from_nanasaki/sylar/http/session/http_session_impl.cc
 */

#include "http_session_impl.h"

namespace sylar {
namespace http {

HttpSessionImpl::HttpSessionImpl(IHttpProtocol::ptr protocol, IHttpTransport::ptr transport)
  : m_protocol(protocol)
  , m_transport(transport) {
}

HttpRequest::ptr HttpSessionImpl::recvRequest() {
  if (!m_protocol || !m_transport) {
    return nullptr;
  }
  return m_protocol->recvRequest(m_transport);
}

int HttpSessionImpl::sendResponse(HttpResponse::ptr response) {
  if (!m_protocol || !m_transport) {
    return -1;
  }
  return m_protocol->sendResponse(m_transport, response);
}

void HttpSessionImpl::close() {
  if (m_transport) {
    m_transport->close();
  }
}

bool HttpSessionImpl::isConnected() {
  if (!m_transport) {
    return false;
  }
  return m_transport->isConnected();
}

std::string HttpSessionImpl::getLocalAddress() {
  if (!m_transport) {
    return "unknown";
  }
  return m_transport->getLocalAddress();
}

std::string HttpSessionImpl::getRemoteAddress() {
  if (!m_transport) {
    return "unknown";
  }
  return m_transport->getRemoteAddress();
}

Socket::ptr HttpSessionImpl::getSocket() {
  if (!m_transport) {
    return nullptr;
  }
  return m_transport->getSocket();
}

}  // namespace http
}  // namespace sylar 