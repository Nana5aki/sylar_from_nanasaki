/*
 * @Author: Nana5aki
 * @Date: 2025-04-06 10:52:29
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-04-06 16:37:39
 * @FilePath: /MySylar/tests/test_socket_tcp_server.cpp
 */
#include "sylar/config.h"
#include "sylar/env.h"
#include "sylar/iomanager.h"
#include "sylar/log.h"
#include "sylar/macro.h"
#include "sylar/socket.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test_tcp_server() {
  int ret;

  auto addr = sylar::Address::LookupAnyIPAddress("0.0.0.0:12345");
  SYLAR_ASSERT(addr);

  auto socket = sylar::Socket::CreateTCPSocket();
  SYLAR_ASSERT(socket);

  ret = socket->bind(addr);
  SYLAR_ASSERT(ret);

  SYLAR_LOG_INFO(g_logger) << "bind success";

  ret = socket->listen();
  SYLAR_ASSERT(ret);

  SYLAR_LOG_INFO(g_logger) << socket->toString();
  SYLAR_LOG_INFO(g_logger) << "listening...";

  while (1) {
    auto client = socket->accept();
    SYLAR_ASSERT(client);
    SYLAR_LOG_INFO(g_logger) << "new client: " << client->toString();
    client->send("hello world", strlen("hello world"));
    client->close();
  }
}

int main(int argc, char* argv[]) {
  sylar::EnvMgr::GetInstance()->init(argc, argv);
  sylar::Config::LoadFromConfDir(sylar::EnvMgr::GetInstance()->getConfigPath());

  sylar::IOManager iom(2);
  iom.schedule(&test_tcp_server);

  return 0;
}