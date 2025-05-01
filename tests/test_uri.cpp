/*
 * @Author: Nana5aki
 * @Date: 2025-04-30 22:19:25
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-05-01 09:01:07
 * @FilePath: /MySylar/tests/test_uri.cpp
 */
#include "sylar/uri.h"
#include <iostream>

int main(int argc, char** argv) {
  sylar::Uri::ptr uri = sylar::Uri::Create("http://www.sylar.top/test/uri?id=100&name=sylar#frg");
  std::cout << uri->toString() << std::endl;
  auto addr = uri->createAddress();
  std::cout << *addr << std::endl;

  // uri = sylar::Uri::Create(
  //   "http://admin@www.sylar.top/test/中文/uri?id=100&name=sylar&vv=中文#frg中文");
  uri = sylar::Uri::Create("http://admin@www.sylar.top/test/%E4%B8%AD%E6%96%87/"
                           "uri?id=100&name=sylar&vv=%E4%B8%AD%E6%96%87#frg%E4%B8%AD%E6%96%87");
  std::cout << uri->toString() << std::endl;
  addr = uri->createAddress();
  std::cout << *addr << std::endl;

  uri = sylar::Uri::Create("http://admin@www.sylar.top");
  std::cout << uri->toString() << std::endl;
  addr = uri->createAddress();
  std::cout << *addr << std::endl;

  uri = sylar::Uri::Create("http://www.sylar.top/test/uri");
  std::cout << uri->toString() << std::endl;
  addr = uri->createAddress();
  std::cout << *addr << std::endl;
  return 0;
}