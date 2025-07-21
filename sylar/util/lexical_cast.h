/*
 * @Author: Nana5aki
 * @Date: 2025-07-19 17:30:00
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-07-20 17:51:01
 * @FilePath: /sylar_from_nanasaki/sylar/util/lexical_cast.h
 */

#pragma once

#include <string>
#include <algorithm>
#include <cctype>
#include <cassert>

namespace sylar {
namespace util {

// 通用 lexical_cast 模板
template<typename Target, typename Source>
Target lexical_cast(const Source& arg) {
    assert(false && "undefined lexical_cast type!");
    return Target();
}

// string 到 string
template<>
inline std::string lexical_cast<std::string, std::string>(const std::string& arg) {
    return arg;
}

// const char* 到 string
template<>
inline std::string lexical_cast<std::string, const char*>(const char* const& arg) {
    return std::string(arg);
}

// string 到 const char*
template<>
inline const char* lexical_cast<const char*, std::string>(const std::string& arg) {
    return arg.c_str();
}

// string 到 bool
template<>
inline bool lexical_cast<bool, std::string>(const std::string& arg) {
    std::string lower = arg;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    if (lower == "true" || lower == "1" || lower == "yes" || lower == "on") return true;
    if (lower == "false" || lower == "0" || lower == "no" || lower == "off") return false;
    return false;
}

// bool 到 string
template<>
inline std::string lexical_cast<std::string, bool>(const bool& arg) {
    return arg ? "true" : "false";
}

// string 到 int
template<>
inline int lexical_cast<int, std::string>(const std::string& arg) {
    return std::stoi(arg);
}

// int 到 string
template<>
inline std::string lexical_cast<std::string, int>(const int& arg) {
    return std::to_string(arg);
}

// string 到 long
template<>
inline long lexical_cast<long, std::string>(const std::string& arg) {
    return std::stol(arg);
}

// long 到 string
template<>
inline std::string lexical_cast<std::string, long>(const long& arg) {
    return std::to_string(arg);
}

// string 到 unsigned int
template<>
inline unsigned int lexical_cast<unsigned int, std::string>(const std::string& arg) {
    return std::stoul(arg);
}

// unsigned int 到 string
template<>
inline std::string lexical_cast<std::string, unsigned int>(const unsigned int& arg) {
    return std::to_string(arg);
}

// string 到 unsigned long
template<>
inline unsigned long lexical_cast<unsigned long, std::string>(const std::string& arg) {
    return std::stoul(arg);
}

// unsigned long 到 string
template<>
inline std::string lexical_cast<std::string, unsigned long>(const unsigned long& arg) {
    return std::to_string(arg);
}

// string 到 float
template<>
inline float lexical_cast<float, std::string>(const std::string& arg) {
    return std::stof(arg);
}

// float 到 string
template<>
inline std::string lexical_cast<std::string, float>(const float& arg) {
    return std::to_string(arg);
}

// string 到 double
template<>
inline double lexical_cast<double, std::string>(const std::string& arg) {
    return std::stod(arg);
}

// double 到 string
template<>
inline std::string lexical_cast<std::string, double>(const double& arg) {
    return std::to_string(arg);
}

}   // namespace util
}   // namespace sylar
