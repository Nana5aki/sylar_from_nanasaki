/*
 * @Author: Nana5aki
 * @Date: 2025-05-01 19:02:36
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-07-13 16:55:11
 * @FilePath: /sylar_from_nanasaki/tests/test_http_server.cpp
 */
#include "sylar/config.h"
#include "sylar/env.h"
#include "sylar/http/http_server.h"
#include "sylar/log.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

#define XX(...) #__VA_ARGS__

sylar::IOManager::ptr worker;

void run() {
  g_logger->setLevel(sylar::LogLevel::INFO);
  // sylar::http::HttpServer::ptr server(
  //   new sylar::http::HttpServer(true, worker.get(), sylar::IOManager::GetThis()));
  sylar::http::HttpServer::ptr server(new sylar::http::HttpServer(true));
  sylar::Address::ptr addr = sylar::Address::LookupAnyIPAddress("0.0.0.0:8020");
  SYLAR_LOG_INFO(g_logger) << "start";
  while (!server->bind(addr)) {
    SYLAR_LOG_INFO(g_logger) << "bind " << *addr << " fail";
    sleep(2);
  }
  auto sd = server->getServletDispatch();
  sd->addServlet("/sylar/xx",
                 [](sylar::http::HttpRequest::ptr req,
                    sylar::http::HttpResponse::ptr rsp,
                    sylar::http::IHttpSession::ptr session) {
                   rsp->setBody(req->toString());
                   return 0;
                 });

  sd->addGlobServlet("/sylar/*",
                     [](sylar::http::HttpRequest::ptr req,
                        sylar::http::HttpResponse::ptr rsp,
                        sylar::http::IHttpSession::ptr) {
                       rsp->setBody("Glob:\r\n" + req->toString());
                       return 0;
                     });

  sd->addGlobServlet(
    "/sylarx/*",
    [](sylar::http::HttpRequest::ptr,
       sylar::http::HttpResponse::ptr rsp,
       sylar::http::IHttpSession::ptr) {
      rsp->setBody(XX(<html><head> < title > 404 Not Found</ title></ head><body><center> < h1 >
                        404 Not Found</ h1></ center><hr><center> nginx / 1.16.0 < / center >
                        </ body></ html> < !--a padding to disable MSIE and
                      Chrome friendly error page-- > < !--a padding to disable MSIE and
                      Chrome friendly error page-- > < !--a padding to disable MSIE and
                      Chrome friendly error page-- > < !--a padding to disable MSIE and
                      Chrome friendly error page-- > < !--a padding to disable MSIE and
                      Chrome friendly error page-- > < !--a padding to disable MSIE and
                      Chrome friendly error page-- >));
      return 0;
    });

  server->start();
}

int main(int argc, char** argv) {
  sylar::EnvMgr::GetInstance()->init(argc, argv);
  sylar::Config::LoadFromConfDir(sylar::EnvMgr::GetInstance()->getConfigPath());

  sylar::IOManager iom(1, true, "main");
  worker.reset(new sylar::IOManager(3, false, "worker"));
  iom.schedule(run);
  return 0;
}