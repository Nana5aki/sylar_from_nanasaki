/*
 * @Author: Nana5aki
 * @Date: 2025-03-27 08:42:33
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-03-27 09:08:59
 * @FilePath: /MySylar/tests/test_hook.cpp
 */
#include "sylar/sylar.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test_sleep() {
  SYLAR_LOG_INFO(g_logger) << "test_sleep begin";
  sylar::IOManager iom;

  /**
   * 这里的两个协程sleep是同时开始的，一共只会睡眠3秒钟，第一个协程开始sleep后，会yield到后台，
   * 第二个协程会得到执行，最终两个协程都会yield到后台，并等待睡眠时间结束，相当于两个sleep是同一起点开始的
   */
  iom.schedule([] {
    sleep(2);
    SYLAR_LOG_INFO(g_logger) << "sleep 2";
  });

  iom.schedule([] {
    sleep(3);
    SYLAR_LOG_INFO(g_logger) << "sleep 3";
  });

  SYLAR_LOG_INFO(g_logger) << "test_sleep end";
}

int main(int argc, char* argv[]) {
  sylar::EnvMgr::GetInstance()->init(argc, argv);
  sylar::Config::LoadFromConfDir(sylar::EnvMgr::GetInstance()->getConfigPath());

  test_sleep();
}