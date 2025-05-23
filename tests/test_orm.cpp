/*
 * @Author: Nana5aki
 * @Date: 2025-05-23 21:22:13
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-05-24 00:19:45
 * @FilePath: /sylar_from_nanasaki/tests/test_orm.cpp
 */

#include "sylar/config.h"
#include "sylar/env.h"

int main(int argc, char* argv[]) {
  sylar::EnvMgr::GetInstance()->init(argc, argv);
  sylar::Config::LoadFromConfDir(sylar::EnvMgr::GetInstance()->getConfigPath());

  return 0;
}