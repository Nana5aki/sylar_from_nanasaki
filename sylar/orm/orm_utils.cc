/*
 * @Author: Nana5aki
 * @Date: 2025-05-09 08:55:52
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-05-18 18:08:25
 * @FilePath: /MySylar/sylar/orm/orm_utils.cc
 */
#include "orm_utils.h"
#include "sylar/util/string_util.h"
#include <sstream>

namespace sylar {
namespace orm {

std::string GetAsVariable(const std::string& v) {
  return sylar::string_util::ToLower(v);
}

std::string GetAsClassName(const std::string& v) {
  auto vs = sylar::string_util::split(v, '_');
  std::stringstream ss;
  for (auto& i : vs) {
    i[0] = toupper(i[0]);
    ss << i;
  }
  return ss.str();
}

std::string GetAsMemberName(const std::string& v) {
  auto class_name = GetAsClassName(v);
  class_name[0] = tolower(class_name[0]);
  return "m_" + class_name;
}

std::string GetAsGetFunName(const std::string& v) {
  auto class_name = GetAsClassName(v);
  return "get" + class_name;
}

std::string GetAsSetFunName(const std::string& v) {
  auto class_name = GetAsClassName(v);
  return "set" + class_name;
}

std::string XmlToString(const tinyxml2::XMLNode& node) {
  return "";
}

std::string GetAsDefineMacro(const std::string& v) {
  std::string tmp = sylar::string_util::replace(v, '.', '_');
  tmp = sylar::string_util::ToUpper(tmp);
  return "__" + tmp + "__";
}

}   // namespace orm
}   // namespace sylar