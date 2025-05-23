/*
 * @Author: Nana5aki
 * @Date: 2025-04-30 22:19:38
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-05-01 09:17:22
 * @FilePath: /sylar_from_nanasaki/tests/test_http_connection.cpp
 */
#include "sylar/http/http_connection.h"
#include "sylar/iomanager.h"
#include "sylar/log.h"
#include <iostream>

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test_pool() {
  sylar::http::HttpConnectionPool::ptr pool =
    sylar::http::HttpConnectionPool::Create("http://www.midlane.top:80", "", 10, 1000 * 30, 5);

  sylar::IOManager::GetThis()->addTimer(
    1000,
    [pool]() {
      auto r = pool->doGet("/", 300);
      std::cout << r->toString() << std::endl;
    },
    true);
}

void run() {
  sylar::Address::ptr addr = sylar::Address::LookupAnyIPAddress("www.midlane.top:80");
  if (!addr) {
    SYLAR_LOG_INFO(g_logger) << "get addr error";
    return;
  }

  sylar::Socket::ptr sock = sylar::Socket::CreateTCP(addr);
  bool rt = sock->connect(addr);
  if (!rt) {
    SYLAR_LOG_INFO(g_logger) << "connect " << *addr << " failed";
    return;
  }

  sylar::http::HttpConnection::ptr conn = std::make_shared<sylar::http::HttpConnection>(sock);
  sylar::http::HttpRequest::ptr req = std::make_shared<sylar::http::HttpRequest>();
  req->setPath("/");
  req->setHeader("host", "www.midlane.top");
  // 小bug，如果设置了keep-alive，那么要在使用前先调用一次init
  req->setHeader("connection", "keep-alive");
  req->init();
  std::cout << "req:" << std::endl << *req << std::endl;

  conn->sendRequest(req);
  auto rsp = conn->recvResponse();

  if (!rsp) {
    SYLAR_LOG_INFO(g_logger) << "recv response error";
    return;
  }
  std::cout << "rsp:" << std::endl << *rsp << std::endl;

  std::cout << "=========================" << std::endl;

  auto r = sylar::http::HttpConnection::DoGet("http://www.midlane.top/wiki/", 300);
  std::cout << "result=" << r->result << " error=" << r->error
            << " rsp=" << (r->response ? r->response->toString() : "") << std::endl;

  std::cout << "=========================" << std::endl;
  test_pool();
}

int main(int argc, char** argv) {
  sylar::IOManager iom(2);
  iom.schedule(run);
  return 0;
}
