/*
 * @Author: Nana5aki
 * @Date: 2025-01-01 14:20:44
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-05-18 18:19:57
 * @FilePath: /MySylar/tests/test_thread.cpp
 */
#include "sylar/config.h"
#include "sylar/env.h"
#include "sylar/log.h"
#include "sylar/mutex.h"
#include "sylar/thread.h"
#include "sylar/util/util.h"

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

int count = 0;
sylar::Mutex s_mutex;

void func1(void* arg) {
  SYLAR_LOG_INFO(g_logger) << "name:" << sylar::Thread::GetName()
                           << " this.name:" << sylar::Thread::GetThis()->getName()
                           << " thread name:" << sylar::util::GetThreadName()
                           << " id:" << sylar::util::GetThreadId()
                           << " this.id:" << sylar::Thread::GetThis()->getId();
  SYLAR_LOG_INFO(g_logger) << "arg: " << *(int*)arg;
  for (int i = 0; i < 10000; i++) {
    sylar::Mutex::Lock lock(s_mutex);
    ++count;
  }
}

int main(int argc, char* argv[]) {
  sylar::EnvMgr::GetInstance()->init(argc, argv);
  sylar::Config::LoadFromConfDir(sylar::EnvMgr::GetInstance()->getConfigPath());

  std::vector<sylar::Thread::ptr> thrs;
  int arg = 123456;
  for (int i = 0; i < 3; i++) {
    // 带参数的线程用std::bind进行参数绑定
    sylar::Thread::ptr thr(
      new sylar::Thread(std::bind(func1, &arg), "thread_" + std::to_string(i)));
    thrs.push_back(thr);
  }

  for (int i = 0; i < 3; i++) {
    thrs[i]->join();
  }

  SYLAR_LOG_INFO(g_logger) << "count = " << count;
  return 0;
}