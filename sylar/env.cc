/*
 * @Author: Nana5aki
 * @Date: 2024-11-30 15:44:46
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-05-18 18:18:01
 * @FilePath: /MySylar/sylar/env.cc
 */
#include "env.h"
#include "config.h"
#include "log.h"
#include <cstdlib>
#include <iomanip>
#include <string.h>
#include <unistd.h>

namespace sylar {

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

bool Env::init(int argc, char** argv) {
  char link[1024] = {0};
  char path[1024] = {0};
  // /proc/pid/exe 一个符号连接->指向进程ID为pid的进程的可执行文件
  sprintf(link, "/proc/%d/exe", getpid());
  // readlink读取符号连接的内容并将其存储在缓冲区
  readlink(link, path, sizeof(path));
  // 上面两行获得程序bin文件绝对路径
  //  /path/xxx/exe
  m_exe = path;

  // 获得当前程序工作目录
  auto pos = m_exe.find_last_of("/");
  m_cwd = m_exe.substr(0, pos) + "/";

  // 程序名字，即文件名
  m_program = argv[0];

  // 分析传入参数（程序环境变量）
  // -config /path/to/config -file xxxx -d
  const char* now_key = nullptr;
  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      if (strlen(argv[i]) > 1) {
        if (now_key) {
          add(now_key, "");
        }
        now_key = argv[i] + 1;
      } else {
        SYLAR_LOG_ERROR(g_logger) << "invalid arg idx=" << i << " val=" << argv[i];
        return false;
      }
    } else {
      if (now_key) {
        add(now_key, argv[i]);
        now_key = nullptr;
      } else {
        SYLAR_LOG_ERROR(g_logger) << "invalid arg idx=" << i << " val=" << argv[i];
        return false;
      }
    }
  }
  if (now_key) {
    add(now_key, "");
  }
  return true;
}

void Env::add(const std::string& key, const std::string& val) {
  RWMutexType::WriteLock lock(m_mutex);
  m_args[key] = val;
}

bool Env::has(const std::string& key) {
  RWMutexType::ReadLock lock(m_mutex);
  auto it = m_args.find(key);
  return it != m_args.end();
}

void Env::del(const std::string& key) {
  RWMutexType::WriteLock lock(m_mutex);
  m_args.erase(key);
}

std::string Env::get(const std::string& key, const std::string& default_value) {
  RWMutexType::ReadLock lock(m_mutex);
  auto it = m_args.find(key);
  return it != m_args.end() ? it->second : default_value;
}

void Env::addHelp(const std::string& key, const std::string& desc) {
  removeHelp(key);
  RWMutexType::WriteLock lock(m_mutex);
  m_helps.push_back(std::make_pair(key, desc));
}

void Env::removeHelp(const std::string& key) {
  RWMutexType::WriteLock lock(m_mutex);
  for (auto it = m_helps.begin(); it != m_helps.end();) {
    if (it->first == key) {
      it = m_helps.erase(it);
    } else {
      ++it;
    }
  }
}

void Env::printHelp() {
  RWMutexType::ReadLock lock(m_mutex);
  std::cout << "Usage: " << m_program << " [options]" << std::endl;
  for (auto& i : m_helps) {
    // setw() 函数用于设置字段的宽度
    // 当后面紧跟着的输出字段长度小于 n 的时候，在该字段前面用空格补齐，当输出字段长度大于 n
    // 时，全部整体输出
    std::cout << std::setw(5) << "-" << i.first << " : " << i.second << std::endl;
  }
}

bool Env::setEnv(const std::string& key, const std::string& val) {
  return !setenv(key.c_str(), val.c_str(), 1);
}

std::string Env::getEnv(const std::string& key, const std::string& default_value) {
  const char* v = getenv(key.c_str());
  if (v == nullptr) {
    return default_value;
  }
  return v;
}

std::string Env::getAbsolutePath(const std::string& path) const {
  if (path.empty()) {
    return "/";
  }
  if (path[0] == '/') {
    return path;
  }
  return m_cwd + path;
}

std::string Env::getConfigPath() {
  return getAbsolutePath(get("c", "conf"));
}

std::string Env::getAbsoluteWorkPath(const std::string& path) const {
  if (path.empty()) {
    return "/";
  }
  if (path[0] == '/') {
    return path;
  }
  static sylar::ConfigVar<std::string>::ptr g_server_work_path =
    sylar::Config::Lookup<std::string>("server.work_path");
  return g_server_work_path->getValue() + "/" + path;
}

}   // namespace sylar
