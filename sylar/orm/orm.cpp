/*
 * @Author: Nana5aki
 * @Date: 2025-05-24 11:37:18
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-05-31 11:50:45
 * @FilePath: /sylar_from_nanasaki/sylar/orm/orm.cpp
 */
#include "sylar/log.h"
#include "sylar/util/fs_util.h"
#include "sylar/util/string_util.h"
#include "table.h"
#include <fstream>
#include <iostream>
#include <map>
#include <vector>

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("orm");

void gen_cmake(const std::string& path, const std::map<std::string, sylar::orm::Table::ptr>& tbs) {
  std::ofstream ofs(path + "/CMakeLists.txt");
  ofs << "cmake_minimum_required(VERSION 3.5)" << std::endl;
  ofs << "project(orm_data)" << std::endl;
  ofs << std::endl;
  ofs << "set(LIB_SRC" << std::endl;
  for (auto& i : tbs) {
    ofs << "    " << sylar::StrUtil::replace(i.second->getNamespace(), ".", "/") << "/"
        << sylar::StrUtil::ToLower(i.second->getFilename()) << ".cc" << std::endl;
  }
  ofs << ")" << std::endl;
  ofs << "add_library(orm_data ${LIB_SRC})" << std::endl;
  ofs << "force_redefine_file_macro_for_sources(orm_data)" << std::endl;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cout << "use as[" << argv[0] << " orm_config_path orm_output_path]" << std::endl;
  }

  std::string out_path = "./orm_out";
  std::string input_path = "./orm";
  if (argc > 1) {
    input_path = argv[1];
  }
  if (argc > 2) {
    out_path = argv[2];
  }
  std::vector<std::string> files;
  sylar::FSUtil::ListAllFile(files, input_path, ".xml");
  std::vector<sylar::orm::Table::ptr> tbs;
  bool has_error = false;
  for (auto& i : files) {
    SYLAR_LOG_INFO(g_logger) << "init xml=" << i << " begin";
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(i.c_str())) {
      SYLAR_LOG_ERROR(g_logger) << "error: " << doc.ErrorStr();
      has_error = true;
    } else {
      sylar::orm::Table::ptr table(new sylar::orm::Table);
      if (!table->init(*doc.RootElement())) {
        SYLAR_LOG_ERROR(g_logger) << "table init error";
        has_error = true;
      } else {
        tbs.push_back(table);
      }
    }
    SYLAR_LOG_INFO(g_logger) << "init xml=" << i << " end";
  }
  if (has_error) {
    return 0;
  }

  std::map<std::string, sylar::orm::Table::ptr> orm_tbs;
  for (auto& i : tbs) {
    i->gen(out_path);
    orm_tbs[i->getName()] = i;
  }
  gen_cmake(out_path, orm_tbs);
  return 0;
}
