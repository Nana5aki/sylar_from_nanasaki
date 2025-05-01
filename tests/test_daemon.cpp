/*
 * @Author: Nana5aki
 * @Date: 2025-05-01 12:44:00
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-05-01 13:01:28
 */
#include "sylar/daemon.h"
#include "sylar/iomanager.h"
#include "sylar/log.h"
#include "sylar/env.h"
#include "sylar/config.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("test");

sylar::Timer::ptr timer;
int server_main(int argc, char** argv) {
  SYLAR_LOG_INFO(g_logger) << sylar::ProcessInfoMgr::GetInstance()->toString();
  sylar::IOManager iom(1);
  timer = iom.addTimer(
    1000,
    []() {
      SYLAR_LOG_INFO(g_logger) << "onTimer";
      static int count = 0;
      if (++count > 10) {
        exit(1);
      }
    },
    true);
  return 0;
}

int main(int argc, char** argv) {
  sylar::EnvMgr::GetInstance()->init(argc, argv);
  sylar::Config::LoadFromConfDir(sylar::EnvMgr::GetInstance()->getConfigPath());
  
  return sylar::start_daemon(argc, argv, server_main, false);
}