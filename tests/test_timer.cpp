/*
 * @Author: Nana5aki
 * @Date: 2025-02-25 23:10:02
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-04-04 10:47:52
 * @FilePath: /sylar_from_nanasaki/tests/test_timer.cpp
 */
/**
 * @file test_timer.cc
 * @brief IO协程测试器定时器测试
 * @version 0.1
 * @date 2021-06-19
 */

#include "sylar/config.h"
#include "sylar/env.h"
#include "sylar/iomanager.h"
#include "sylar/log.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

static int timeout = 1000;
static sylar::Timer::ptr s_timer;

void timer_callback() {
  SYLAR_LOG_INFO(g_logger) << "timer callback, timeout = " << timeout;
  timeout += 1000;
  if (timeout < 5000) {
    s_timer->reset(timeout, true);
  } else {
    s_timer->cancel();
  }
}

void test_timer() {
  sylar::IOManager iom;

  // 循环定时器
  s_timer = iom.addTimer(1000, timer_callback, true);

  // 单次定时器
  iom.addTimer(500, [] { SYLAR_LOG_INFO(g_logger) << "500ms timeout"; });
  iom.addTimer(5000, [] { SYLAR_LOG_INFO(g_logger) << "5000ms timeout"; });
}

int main(int argc, char* argv[]) {
  sylar::EnvMgr::GetInstance()->init(argc, argv);
  sylar::Config::LoadFromConfDir(sylar::EnvMgr::GetInstance()->getConfigPath());

  test_timer();

  SYLAR_LOG_INFO(g_logger) << "end";

  return 0;
}