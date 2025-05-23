/*
 * @Author: Nana5aki
 * @Date: 2025-05-09 08:55:46
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-05-21 09:07:22
 * @FilePath: /sylar_from_nanasaki/sylar/orm/orm_utils.h
 */
#pragma once

#include <string>
#include <tinyxml2.h>

namespace sylar {
namespace orm {

/**
 * @brief 将字符串转换为变量名格式（全小写）
 * @param v 输入字符串
 * @return 转换后的变量名字符串
 */
std::string GetAsVariable(const std::string& v);

/**
 * @brief 将字符串转换为类名格式（驼峰命名）
 * @param v 输入字符串，以下划线分隔
 * @return 转换后的类名字符串
 */
std::string GetAsClassName(const std::string& v);

/**
 * @brief 将字符串转换为成员变量名格式（m_前缀+驼峰命名）
 * @param v 输入字符串
 * @return 转换后的成员变量名字符串
 */
std::string GetAsMemberName(const std::string& v);

/**
 * @brief 生成getter函数名
 * @param v 输入字符串
 * @return getter函数名字符串
 */
std::string GetAsGetFunName(const std::string& v);

/**
 * @brief 生成setter函数名
 * @param v 输入字符串
 * @return setter函数名字符串
 */
std::string GetAsSetFunName(const std::string& v);

/**
 * @brief 将XML节点转换为字符串
 * @param node XML节点
 * @return 转换后的字符串
 */
std::string XmlToString(const tinyxml2::XMLNode& node);

/**
 * @brief 将字符串转换为宏定义格式
 * @param v 输入字符串
 * @return 转换后的宏定义字符串（全大写，点号替换为下划线，添加双下划线前缀和后缀）
 */
std::string GetAsDefineMacro(const std::string& v);

}   // namespace orm
}   // namespace sylar