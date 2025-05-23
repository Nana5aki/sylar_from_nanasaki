/*
 * @Author: Nana5aki
 * @Date: 2025-05-10 15:30:26
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-05-18 17:59:23
 * @FilePath: /sylar_from_nanasaki/sylar/util/string_util.h
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

/**
 * @brief url编码
 * @param[in] str 原始字符串
 * @param[in] space_as_plus 是否将空格编码成+号，如果为false，则空格编码成%20
 * @return 编码后的字符串
 */
std::string UrlEncode(const std::string& str, bool space_as_plus = true);

/**
 * @brief url解码
 * @param[in] str url字符串
 * @param[in] space_as_plus 是否将+号解码为空格
 * @return 解析后的字符串
 */
std::string UrlDecode(const std::string& str, bool space_as_plus = true);

/**
 * @brief 移除字符串首尾的指定字符串
 * @param[] str 输入字符串
 * @param[] delimit 待移除的字符串
 * @return  移除后的字符串
 */
std::string Trim(const std::string& str, const std::string& delimit = " \t\r\n");

/**
 * @brief 移除字符串首部的指定字符串
 * @param[] str 输入字符串
 * @param[] delimit 待移除的字符串
 * @return  移除后的字符串
 */
std::string TrimLeft(const std::string& str, const std::string& delimit = " \t\r\n");

/**
 * @brief 移除字符尾部的指定字符串
 * @param[] str 输入字符串
 * @param[] delimit 待移除的字符串
 * @return  移除后的字符串
 */
std::string TrimRight(const std::string& str, const std::string& delimit = " \t\r\n");


std::string ToUpper(const std::string& name);

std::string ToLower(const std::string& name);

}   // namespace string_util
}   // namespace sylar