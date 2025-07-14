/*
 * @Author: Nana5aki
 * @Date: 2025-04-26 15:17:16
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-07-15 00:16:15
 * @FilePath: /sylar_from_nanasaki/tests/test_http.cpp
 */
#include "sylar/http/core/http_request.h"
#include "sylar/http/core/http_response.h"
#include <iostream>

void test_request() {
  sylar::http::HttpRequest::ptr req(new sylar::http::HttpRequest);
  req->setHeader("host", "www.baidu.com");
  req->setBody("testtest");
  req->dump(std::cout) << std::endl;
}

void test_response() {
  sylar::http::HttpResponse::ptr rsp(new sylar::http::HttpResponse);
  rsp->setHeader("X-X", "TEST");
  rsp->setBody("testtest");
  rsp->setStatus((sylar::http::HttpStatus)400);
  rsp->setClose(false);

  rsp->dump(std::cout) << std::endl;
}

int main(int argc, char** argv) {
  test_request();
  test_response();
  return 0;
}