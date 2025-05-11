/*
 * @Author: Nana5aki
 * @Date: 2025-05-10 15:30:38
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-05-10 17:02:13
 * @FilePath: /MySylar/sylar/util/string_util.cc
 */
#include "string_util.h"
#include <algorithm>
#include <sstream>

namespace sylar {
namespace string_util {

std::vector<std::string> split(const std::string& str, char delim, size_t max) {
  if (str.empty()) {
    return {};
  }
  std::vector<std::string> result;
  size_t last = 0;
  size_t pos = str.find(delim);
  while (pos != std::string::npos) {
    result.push_back(str.substr(last, pos - last));
    last = pos + 1;
    if (--max == 1) break;
    pos = str.find(delim, last);
  }
  result.push_back(str.substr(last));
  return result;
}

std::vector<std::string> split(const std::string& str, const char* delims, size_t max) {
  if (str.empty() || !delims) {
    return {};
  }

  std::vector<std::string> result;

  size_t last = 0;
  size_t pos = str.find_first_of(delims);

  while (pos != std::string::npos && max > 1) {
    result.emplace_back(str.substr(last, pos - last));
    last = pos + 1;
    --max;
    pos = str.find_first_of(delims, last);
  }

  result.emplace_back(str.substr(last));
  return result;
}

std::string replace(const std::string& str, char find, char replaceWith) {
  if (str.empty()) {
    return str;
  }
  std::string result(str);
  std::replace(result.begin(), result.end(), find, replaceWith);
  return result;
}

std::string replace(const std::string& str, char find, const std::string& replaceWith) {
  if (str.empty() || replaceWith.empty()) {
    return str;
  }
  std::stringstream ss;
  for (const auto& c : str) {
    if (c == find) {
      ss << replaceWith;
    } else {
      ss << c;
    }
  }

  return ss.str();
}

std::string replace(const std::string& str, const std::string& find,
                    const std::string& replaceWith) {
  if (str.empty() || find.empty()) {
    return str;
  }
  std::string result;
  size_t last = 0;
  size_t pos = str.find(find);
  while (pos != std::string::npos) {
    result.append(str.substr(last, pos - last));
    result.append(replaceWith);
    last = pos + find.length();
    pos = str.find(find, last);
  }
  result.append(str.substr(last));
  return result;
}

}   // namespace string_util
}   // namespace sylar
