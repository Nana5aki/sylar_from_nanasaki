/*
 * @Author: Nana5aki
 * @Date: 2025-05-10 19:19:49
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-05-10 22:06:10
 * @FilePath: /sylar_from_nanasaki/sylar/orm/index.h
 */
#pragma once

#include "tinyxml2.h"
#include <memory>
#include <string>
#include <vector>

namespace sylar {
namespace orm {

/**
 * @brief 数据库索引定义类
 * @details 用于定义数据库表的索引属性，包括索引名、类型、列等信息
 * 支持从XML配置文件中解析索引定义
 * 支持主键、唯一索引和普通索引类型
 */
class Index {
public:
  /**
   * @brief 索引类型枚举
   * @details 定义了支持的索引类型：主键、唯一索引和普通索引
   */
  enum Type {
    TYPE_NULL = 0,   ///< 空类型，表示未定义类型
    TYPE_PK,         ///< 主键索引，对应"pk"
    TYPE_UNIQ,       ///< 唯一索引，对应"uniq"
    TYPE_INDEX       ///< 普通索引，对应"index"
  };

  using ptr = std::shared_ptr<Index>;

public:
  /**
   * @brief 获取索引名称
   * @return 索引名称字符串
   */
  const std::string& getName() const {
    return m_name;
  }

  /**
   * @brief 获取索引类型字符串
   * @return 类型字符串，如"pk"、"uniq"、"index"
   */
  const std::string& getType() const {
    return m_type;
  }

  /**
   * @brief 获取索引描述
   * @return 描述字符串，用于说明索引的用途
   */
  const std::string& getDesc() const {
    return m_desc;
  }

  /**
   * @brief 获取索引包含的列名列表
   * @return 列名列表，以逗号分隔的列名被解析为vector
   */
  const std::vector<std::string>& getCols() const {
    return m_cols;
  }

  /**
   * @brief 获取索引类型枚举值
   * @return 索引类型枚举，用于类型判断和转换
   */
  Type getDType() const {
    return m_dtype;
  }

  /**
   * @brief 判断是否为主键索引
   * @return 是否为主键索引
   */
  bool isPK() const {
    return m_type == "pk";
  }

  /**
   * @brief 从XML节点初始化索引信息
   * @param node XML节点，包含索引的定义信息
   * @return 是否初始化成功
   * @details 解析XML节点中的name、type、cols、desc等属性
   * 要求name和type属性必须存在
   * cols属性以逗号分隔的列名列表
   */
  bool init(const tinyxml2::XMLElement& node);

public:
  /**
   * @brief 将字符串类型转换为Type枚举值
   * @param v 类型字符串，如"pk"、"uniq"、"index"
   * @return 对应的Type枚举值
   */
  static Type ParseType(const std::string& v);

  /**
   * @brief 将Type枚举值转换为字符串
   * @param v 类型枚举值
   * @return 对应的类型字符串，如"pk"、"uniq"、"index"
   */
  static std::string TypeToString(Type v);

private:
  std::string m_name;                ///< 索引名称，用于标识索引
  std::string m_type;                ///< 索引类型字符串，如"pk"、"uniq"、"index"
  std::string m_desc;                ///< 索引描述，用于说明索引的用途
  std::vector<std::string> m_cols;   ///< 索引包含的列名列表
  Type m_dtype;                      ///< 索引类型枚举，用于类型判断和转换
};

}   // namespace orm
}   // namespace sylar