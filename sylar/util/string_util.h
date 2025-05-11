/*
 * @Author: Nana5aki
 * @Date: 2025-05-10 15:30:26
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-05-10 15:33:58
 * @FilePath: /MySylar/sylar/util/string_util.h
 */
#pragma once

#include <string>
#include <vector>
namespace sylar {
namespace string_util {

std::vector<std::string> split(const std::string& str, char delim, size_t max = ~0);
std::vector<std::string> split(const std::string& str, const char* delims, size_t max = ~0);

std::string replace(const std::string& str, char find, char replaceWith);
std::string replace(const std::string& str, char find, const std::string& replaceWith);
std::string replace(const std::string& str, const std::string& find,
                    const std::string& replaceWith);

}   // namespace string_util
}   // namespace sylar