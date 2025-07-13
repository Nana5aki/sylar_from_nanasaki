/*
 * @Author: Nana5aki
 * @Date: 2025-07-13 15:20:00
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-07-13 15:20:00
 * @FilePath: /sylar_from_nanasaki/sylar/http/transport/tcp_transport.cc
 */

#include "tcp_transport.h"

namespace sylar {
namespace http {

TcpTransport::TcpTransport(Socket::ptr socket, bool owner)
  : m_sockStream(std::make_shared<SocketStream>(socket, owner)) {
}

TcpTransport::TcpTransport(SocketStream::ptr sockStream)
  : m_sockStream(sockStream) {
}

int TcpTransport::read(void* buffer, size_t length) {
  if (!m_sockStream) {
    return -1;
  }
  return m_sockStream->read(buffer, length);
}

int TcpTransport::write(const void* buffer, size_t length) {
  if (!m_sockStream) {
    return -1;
  }
  return m_sockStream->write(buffer, length);
}

int TcpTransport::writeFixSize(const void* buffer, size_t length) {
  if (!m_sockStream) {
    return -1;
  }
  return m_sockStream->writeFixSize(buffer, length);
}

void TcpTransport::close() {
  if (m_sockStream) {
    m_sockStream->close();
  }
}

bool TcpTransport::isConnected() {
  if (!m_sockStream) {
    return false;
  }
  return m_sockStream->isConnected();
}

std::string TcpTransport::getLocalAddress() {
  if (!m_sockStream) {
    return "unknown";
  }
  auto socket = m_sockStream->getSocket();
  if (!socket || !socket->getLocalAddress()) {
    return "unknown";
  }
  return socket->getLocalAddress()->toString();
}

std::string TcpTransport::getRemoteAddress() {
  if (!m_sockStream) {
    return "unknown";
  }
  auto socket = m_sockStream->getSocket();
  if (!socket || !socket->getRemoteAddress()) {
    return "unknown";
  }
  return socket->getRemoteAddress()->toString();
}

Socket::ptr TcpTransport::getSocket() {
  if (!m_sockStream) {
    return nullptr;
  }
  return m_sockStream->getSocket();
}

SocketStream::ptr TcpTransport::getSocketStream() {
  return m_sockStream;
}

}  // namespace http
}  // namespace sylar 