/*
 * @Author: Nana5aki
 * @Date: 2025-04-13 14:46:23
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-04-25 23:01:05
 * @FilePath: /MySylar/sylar/streams/sock_stream.cc
 */
#include "sock_stream.h"
#include "sylar/socket.h"

namespace sylar {

SocketStream::SocketStream(Socket::ptr sock, bool owner)
  : m_socket(sock)
  , m_owner(owner) {
}

SocketStream::~SocketStream() {
  if (m_owner && m_socket) {
    m_socket->close();
  }
}

bool SocketStream::isConnected() const {
  return m_socket && m_socket->isConnected();
}

int SocketStream::read(void* buffer, size_t length) {
  if (!isConnected()) {
    return -1;
  }
  return m_socket->recv(buffer, length);
}

int SocketStream::read(ByteArray::ptr ba, size_t length) {
  if (!isConnected()) {
    return -1;
  }
  std::vector<iovec> iovs;
  ba->getWriteBuffers(iovs, length);
  int rt = m_socket->recv(&iovs[0], iovs.size());
  if (rt > 0) {
    ba->setPosition(ba->getPosition() + rt);
  }
  return rt;
}

int SocketStream::write(const void* buffer, size_t length) {
  if (!isConnected()) {
    return -1;
  }
  return m_socket->send(buffer, length);
}

int SocketStream::write(ByteArray::ptr ba, size_t length) {
  if (!isConnected()) {
    return -1;
  }
  std::vector<iovec> iovs;
  ba->getReadBuffers(iovs, length);
  int rt = m_socket->send(&iovs[0], iovs.size());
  if (rt > 0) {
    ba->setPosition(ba->getPosition() + rt);
  }
  return rt;
}

Address::ptr SocketStream::getRemoteAddress() {
  if (m_socket) {
    return m_socket->getRemoteAddress();
  }
  return nullptr;
}

Address::ptr SocketStream::getLocalAddress() {
  if (m_socket) {
    return m_socket->getLocalAddress();
  }
  return nullptr;
}

std::string SocketStream::getRemoteAddressString() {
  auto addr = getRemoteAddress();
  if (addr) {
    return addr->toString();
  }
  return "";
}

std::string SocketStream::getLocalAddressString() {
  auto addr = getLocalAddress();
  if (addr) {
    return addr->toString();
  }
  return "";
}

void SocketStream::close() {
  if (m_socket) {
    m_socket->close();
  }
}

}   // namespace sylar