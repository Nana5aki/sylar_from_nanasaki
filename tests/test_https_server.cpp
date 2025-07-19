/*
 * @Author: Nana5aki
 * @Date: 2025-07-19 15:21:03
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-07-19 19:14:00
 * @FilePath: /sylar_from_nanasaki/tests/test_https_server.cpp
 */


/// openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -days 365 -nodes -subj "/C=CN/ST=Hubei/L=Wuhan/O=Nanasaki/CN=localhost"
/// openssl x509 -outform der -in cert.pem -out cert.crt

#include "sylar/config.h"
#include "sylar/env.h"
#include "sylar/http/https_server.h"
#include "sylar/log.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

#define XX(...) #__VA_ARGS__

sylar::IOManager::ptr worker;

sylar::ConfigVar<std::string>::ptr cert_file =
  sylar::Config::Lookup("https.cert", std::string(""), "cert file");
sylar::ConfigVar<std::string>::ptr key_file =
  sylar::Config::Lookup("https.key", std::string(""), "key file");

void run() {
  g_logger->setLevel(sylar::LogLevel::DEBUG);
  SYLAR_LOG_INFO(g_logger) << "cert_file: " << cert_file->getValue();
  SYLAR_LOG_INFO(g_logger) << "key_file: " << key_file->getValue();
  sylar::http::HttpsServer::ptr server =
    std::make_shared<sylar::http::HttpsServer>(cert_file->getValue(),
                                               key_file->getValue(),
                                               true,
                                               worker.get(),
                                               sylar::IOManager::GetThis(),
                                               sylar::IOManager::GetThis());
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