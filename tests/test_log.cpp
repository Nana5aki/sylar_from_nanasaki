/*
 * @Author: Nana5aki
 * @Date: 2024-12-21 19:49:21
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-05-18 18:19:30
 * @FilePath: /sylar_from_nanasaki/tests/test_log.cpp
 */
#include "sylar/config.h"
#include "sylar/env.h"
#include "sylar/log.h"
#include "sylar/util/util.h"
#include <unistd.h>

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();   // 默认INFO级别

int main(int argc, char* argv[]) {
  sylar::EnvMgr::GetInstance()->init(argc, argv);
  sylar::Config::LoadFromConfDir(sylar::EnvMgr::GetInstance()->getConfigPath());

  SYLAR_LOG_FATAL(g_logger) << "fatal msg";
  SYLAR_LOG_ERROR(g_logger) << "err msg";
  SYLAR_LOG_INFO(g_logger) << "info msg";
  SYLAR_LOG_DEBUG(g_logger) << "debug msg";

  SYLAR_LOG_FMT_FATAL(g_logger, "fatal %s:%d", __FILE__, __LINE__);
  SYLAR_LOG_FMT_ERROR(g_logger, "err %s:%d", __FILE__, __LINE__);
  SYLAR_LOG_FMT_INFO(g_logger, "info %s:%d", __FILE__, __LINE__);
  SYLAR_LOG_FMT_DEBUG(g_logger, "debug %s:%d", __FILE__, __LINE__);

  sleep(1);
  sylar::util::SetThreadName("brand_new_thread");

  g_logger->setLevel(sylar::LogLevel::WARN);
  SYLAR_LOG_FATAL(g_logger) << "fatal msg";
  SYLAR_LOG_ERROR(g_logger) << "err msg";
  SYLAR_LOG_INFO(g_logger) << "info msg";     // 不打印
  SYLAR_LOG_DEBUG(g_logger) << "debug msg";   // 不打印


  sylar::FileLogAppender::ptr fileAppender(new sylar::FileLogAppender("./log.txt"));
  g_logger->addAppender(fileAppender);
  SYLAR_LOG_FATAL(g_logger) << "fatal msg";
  SYLAR_LOG_ERROR(g_logger) << "err msg";
  SYLAR_LOG_INFO(g_logger) << "info msg";     // 不打印
  SYLAR_LOG_DEBUG(g_logger) << "debug msg";   // 不打印

  sylar::Logger::ptr test_logger = SYLAR_LOG_NAME("test_logger");
  sylar::StdoutLogAppender::ptr appender(new sylar::StdoutLogAppender);
  sylar::LogFormatter::ptr formatter(new sylar::LogFormatter(
    "%d:%rms%T%p%T%c%T%f:%l %m%n"));   // 时间：启动毫秒数 级别 日志名称 文件名：行号 消息 换行
  appender->setFormatter(formatter);
  test_logger->addAppender(appender);
  test_logger->setLevel(sylar::LogLevel::WARN);

  SYLAR_LOG_ERROR(test_logger) << "err msg";
  SYLAR_LOG_INFO(test_logger) << "info msg";   // 不打印

  // 输出全部日志器的配置
  g_logger->setLevel(sylar::LogLevel::INFO);
  SYLAR_LOG_INFO(g_logger) << "logger config:" << sylar::LoggerMgr::GetInstance()->toYamlString();

  return 0;
}