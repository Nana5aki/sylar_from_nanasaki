/*
 * @Author: Nana5aki
 * @Date: 2025-05-01 21:44:37
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-05-07 23:40:30
 * @FilePath: /MySylar/tests/test_tcp_server.cpp
 */
#include "sylar/tcp_server.h"
#include "sylar/log.h"
#include "sylar/macro.h"
#include "sylar/env.h"
#include "sylar/config.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

/**
 * @brief 自定义TcpServer类，重载handleClient方法
 */
class MyTcpServer : public sylar::TcpServer {
protected:
  virtual void handleClient(sylar::Socket::ptr client) override;
};

void MyTcpServer::handleClient(sylar::Socket::ptr client) {
  SYLAR_LOG_INFO(g_logger) << "new client: " << client->toString();
  static std::string buf;
  buf.resize(4096);
  client->recv(&buf[0],
               buf.length());   // 这里有读超时，由tcp_server.read_timeout配置项进行配置，默认120秒
  SYLAR_LOG_INFO(g_logger) << "recv: " << buf;
  client->close();
}

void run() {
  sylar::TcpServer::ptr server(
    new MyTcpServer);   // 内部依赖shared_from_this()，所以必须以智能指针形式创建对象
  auto addr = sylar::Address::LookupAny("0.0.0.0:8020");
  SYLAR_ASSERT(addr);
  std::vector<sylar::Address::ptr> addrs;
  addrs.push_back(addr);

  std::vector<sylar::Address::ptr> fails;
  while (!server->bind(addrs, fails)) {
    sleep(2);
  }

  SYLAR_LOG_INFO(g_logger) << "bind success, " << server->toString();

  server->start();
}

int main(int argc, char* argv[]) {
  sylar::EnvMgr::GetInstance()->init(argc, argv);
  sylar::Config::LoadFromConfDir(sylar::EnvMgr::GetInstance()->getConfigPath());

  sylar::IOManager iom(2);
  iom.schedule(&run);

  return 0;
}