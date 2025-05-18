/*
 * @Author: Nana5aki
 * @Date: 2025-05-10 15:30:38
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-05-18 18:00:06
 * @FilePath: /MySylar/sylar/util/string_util.cc
 */
#include "string_util.h"
#include <algorithm>
#include <sstream>
#include <cstdint>
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

// clang-format off
static const char uri_chars[256] = {
    /* 0 */
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 1, 1, 0,
    1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 0, 0, 0, 1, 0, 0,
    /* 64 */
    0, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 0, 0, 0, 0, 1,
    0, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 0, 0, 0, 1, 0,
    /* 128 */
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    /* 192 */
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
};

static const char xdigit_chars[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0,
    0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

// clang-format on

#define CHAR_IS_UNRESERVED(c) (uri_chars[(unsigned char)(c)])

//-.0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz~
std::string UrlEncode(const std::string& str, bool space_as_plus) {
  static const char* hexdigits = "0123456789ABCDEF";
  std::string* ss = nullptr;
  const char* end = str.c_str() + str.length();
  for (const char* c = str.c_str(); c < end; ++c) {
    if (!CHAR_IS_UNRESERVED(*c)) {
      if (!ss) {
        ss = new std::string;
        ss->reserve(str.size() * 1.2);
        ss->append(str.c_str(), c - str.c_str());
      }
      if (*c == ' ' && space_as_plus) {
        ss->append(1, '+');
      } else {
        ss->append(1, '%');
        ss->append(1, hexdigits[(uint8_t)*c >> 4]);
        ss->append(1, hexdigits[*c & 0xf]);
      }
    } else if (ss) {
      ss->append(1, *c);
    }
  }
  if (!ss) {
    return str;
  } else {
    std::string rt = *ss;
    delete ss;
    return rt;
  }
}

std::string UrlDecode(const std::string& str, bool space_as_plus) {
  std::string* ss = nullptr;
  const char* end = str.c_str() + str.length();
  for (const char* c = str.c_str(); c < end; ++c) {
    if (*c == '+' && space_as_plus) {
      if (!ss) {
        ss = new std::string;
        ss->append(str.c_str(), c - str.c_str());
      }
      ss->append(1, ' ');
    } else if (*c == '%' && (c + 2) < end && isxdigit(*(c + 1)) && isxdigit(*(c + 2))) {
      if (!ss) {
        ss = new std::string;
        ss->append(str.c_str(), c - str.c_str());
      }
      ss->append(1, (char)(xdigit_chars[(int)*(c + 1)] << 4 | xdigit_chars[(int)*(c + 2)]));
      c += 2;
    } else if (ss) {
      ss->append(1, *c);
    }
  }
  if (!ss) {
    return str;
  } else {
    std::string rt = *ss;
    delete ss;
    return rt;
  }
}

std::string Trim(const std::string& str, const std::string& delimit) {
  auto begin = str.find_first_not_of(delimit);
  if (begin == std::string::npos) {
    return "";
  }
  auto end = str.find_last_not_of(delimit);
  return str.substr(begin, end - begin + 1);
}

std::string TrimLeft(const std::string& str, const std::string& delimit) {
  auto begin = str.find_first_not_of(delimit);
  if (begin == std::string::npos) {
    return "";
  }
  return str.substr(begin);
}

std::string TrimRight(const std::string& str, const std::string& delimit) {
  auto end = str.find_last_not_of(delimit);
  if (end == std::string::npos) {
    return "";
  }
  return str.substr(0, end);
}

std::string ToUpper(const std::string& name) {
  std::string rt = name;
  std::transform(rt.begin(), rt.end(), rt.begin(), ::toupper);
  return rt;
}

std::string ToLower(const std::string& name) {
  std::string rt = name;
  std::transform(rt.begin(), rt.end(), rt.begin(), ::tolower);
  return rt;
}

}   // namespace string_util
}   // namespace sylar
