/*
 * @Author: Nana5aki
 * @Date: 2025-04-06 10:52:15
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-04-06 17:12:30
 * @FilePath: /MySylar/tests/test_socket_tcp_client.cpp
 */
#include "sylar/config.h"
#include "sylar/env.h"
#include "sylar/iomanager.h"
#include "sylar/log.h"
#include "sylar/macro.h"
#include "sylar/socket.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void TestTcpClient() {
  int ret;

  auto socket = sylar::Socket::CreateTCPSocket();
  SYLAR_ASSERT(socket);

  auto addr = sylar::Address::LookupAnyIPAddress("0.0.0.0:12345");
  SYLAR_ASSERT(addr);

  ret = socket->connect(addr);
  SYLAR_ASSERT(ret);

  SYLAR_LOG_INFO(g_logger) << "connect success, peer address: "
                           << socket->getRemoteAddress()->toString();

  std::string buffer;
  buffer.resize(1024);
  socket->recv(&buffer[0], buffer.size());
  SYLAR_LOG_INFO(g_logger) << "recv: " << buffer;
  socket->close();

  return;
}

int main(int argc, char* argv[]) {
  sylar::EnvMgr::GetInstance()->init(argc, argv);
  sylar::Config::LoadFromConfDir(sylar::EnvMgr::GetInstance()->getConfigPath());

  sylar::IOManager iom;
  iom.schedule(&TestTcpClient);

  return 0;
}