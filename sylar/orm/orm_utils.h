/*
 * @Author: Nana5aki
 * @Date: 2025-05-09 08:55:46
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-05-10 12:11:06
 * @FilePath: /MySylar/sylar/orm/orm_utils.h
 */
#pragma once

#include <string>
#include <tinyxml2.h>

namespace sylar {
namespace orm {

std::string GetAsVariable(const std::string& v);
std::string GetAsClassName(const std::string& v);
std::string GetAsMemberName(const std::string& v);
std::string GetAsGetFunName(const std::string& v);
std::string GetAsSetFunName(const std::string& v);
std::string XmlToString(const tinyxml2::XMLNode& node);
std::string GetAsDefineMacro(const std::string& v);

}   // namespace orm
}   // namespace sylar