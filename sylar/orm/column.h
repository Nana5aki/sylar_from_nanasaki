/*
 * @Author: Nana5aki
 * @Date: 2025-05-08 23:53:14
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-05-10 23:53:40
 * @FilePath: /MySylar/sylar/orm/column.h
 */
#pragma once

#include <memory>
#include <tinyxml2.h>

namespace sylar {
namespace orm {

/**
 * @brief 数据库列定义类
 * @details 用于定义数据库表的列属性，包括列名、类型、默认值等信息
 * 支持SQLite3和MySQL数据库类型映射
 * 支持从XML配置文件中解析列定义
 * 支持生成C++类成员变量和访问函数
 */
class Column {
  friend class Table;

public:
  using ptr = std::shared_ptr<Column>;

  /**
   * @brief 列数据类型枚举
   * @details 定义了支持的基本数据类型，包括整数、浮点数、字符串、二进制和时间戳类型
   */
  enum Type {
    TYPE_NULL = 0,   ///< 空类型，表示未定义类型
    TYPE_INT8,       ///< 8位有符号整数，对应C++ int8_t
    TYPE_UINT8,      ///< 8位无符号整数，对应C++ uint8_t
    TYPE_INT16,      ///< 16位有符号整数，对应C++ int16_t
    TYPE_UINT16,     ///< 16位无符号整数，对应C++ uint16_t
    TYPE_INT32,      ///< 32位有符号整数，对应C++ int32_t
    TYPE_UINT32,     ///< 32位无符号整数，对应C++ uint32_t
    TYPE_FLOAT,      ///< 单精度浮点数，对应C++ float
    TYPE_DOUBLE,     ///< 双精度浮点数，对应C++ double
    TYPE_INT64,      ///< 64位有符号整数，对应C++ int64_t
    TYPE_UINT64,     ///< 64位无符号整数，对应C++ uint64_t
    TYPE_STRING,     ///< 字符串类型，对应C++ std::string，在MySQL中映射为varchar
    TYPE_TEXT,       ///< 文本类型，对应C++ std::string，在MySQL中映射为text
    TYPE_BLOB,       ///< 二进制大对象，对应C++ std::string，在MySQL中映射为blob
    TYPE_TIMESTAMP   ///< 时间戳类型，对应C++ int64_t，在MySQL中映射为timestamp
  };

  /**
   * @brief 获取列名
   * @return 列名字符串
   */
  const std::string& getName() const {
    return m_name;
  }

  /**
   * @brief 获取列类型字符串
   * @return 类型字符串，如"int32_t"、"std::string"等
   */
  const std::string& getType() const {
    return m_type;
  }

  /**
   * @brief 获取列描述
   * @return 描述字符串，用于说明列的用途
   */
  const std::string& getDesc() const {
    return m_desc;
  }

  /**
   * @brief 获取默认值
   * @return 默认值字符串，根据类型不同返回不同格式的默认值
   */
  const std::string& getDefault() const {
    return m_default;
  }

  /**
   * @brief 获取列索引
   * @return 列索引值，用于标识列在表中的位置
   */
  int getIndex() const {
    return m_index;
  }

  /**
   * @brief 获取更新值
   * @return 更新值字符串，用于指定列的更新规则
   */
  const std::string& getUpdate() const {
    return m_update;
  }

  /**
   * @brief 获取数据类型枚举值
   * @return 数据类型枚举，用于类型判断和转换
   */
  Type getDType() const {
    return m_dtype;
  }

  /**
   * @brief 获取数据类型字符串
   * @return 数据类型字符串，如"int32_t"、"std::string"等
   */
  std::string getDTypeString() {
    return TypeToString(m_dtype);
  }

  /**
   * @brief 判断是否为自增列
   * @return 是否为自增列，用于标识主键自增特性
   */
  bool isAutoIncrement() const {
    return m_autoIncrement;
  }

  /**
   * @brief 获取默认值字符串
   * @return 默认值字符串，根据类型不同返回不同格式的默认值
   * @details 数值类型直接返回默认值
   * 字符串类型添加双引号
   * 时间戳类型特殊处理
   */
  std::string getDefaultValueString();

  /**
   * @brief 获取SQLite3默认值
   * @return SQLite3默认值字符串
   * @details 数值类型默认为"0"
   * 字符串类型默认为"''"
   * 时间戳类型默认为"'1980-01-01 00:00:00'"
   */
  std::string getSQLite3Default();

  /**
   * @brief 从XML节点初始化列信息
   * @param node XML节点，包含列的定义信息
   * @return 是否初始化成功
   * @details 解析XML节点中的name、type、desc、default、update等属性
   */
  bool init(const tinyxml2::XMLElement& node);

  /**
   * @brief 获取成员变量定义
   * @return 成员变量定义字符串，用于生成C++类定义
   */
  std::string getMemberDefine() const;

  /**
   * @brief 获取获取函数定义
   * @return 获取函数定义字符串，用于生成C++类定义
   */
  std::string getGetFunDefine() const;

  /**
   * @brief 获取设置函数定义
   * @return 设置函数定义字符串，用于生成C++类定义
   */
  std::string getSetFunDefine() const;

  /**
   * @brief 获取设置函数实现
   * @param class_name 类名，用于生成完整的函数实现
   * @param idx 索引，用于生成函数体
   * @return 设置函数实现字符串
   */
  std::string getSetFunImpl(const std::string& class_name, int idx) const;

  /**
   * @brief 将字符串类型转换为Type枚举值
   * @param v 类型字符串，如"int32_t"、"std::string"等
   * @return 对应的Type枚举值
   * @details 支持多种类型名称的映射，如int8_t/int8都映射到TYPE_INT8
   */
  static Type ParseType(const std::string& v);

  /**
   * @brief 将Type枚举值转换为字符串
   * @param type 类型枚举值
   * @return 对应的类型字符串，如"int32_t"、"std::string"等
   */
  static std::string TypeToString(Type type);

  /**
   * @brief 获取SQLite3类型字符串
   * @return SQLite3类型字符串，如"INTEGER"、"TEXT"等
   * @details 将内部类型映射到SQLite3支持的类型
   */
  std::string getSQLite3TypeString();

  /**
   * @brief 获取MySQL类型字符串
   * @return MySQL类型字符串，如"int"、"varchar(128)"等
   * @details 将内部类型映射到MySQL支持的类型
   */
  std::string getMySQLTypeString();

  /**
   * @brief 获取绑定函数名
   * @return 绑定函数名字符串，如"bindInt32"、"bindString"等
   * @details 用于生成数据库绑定函数名
   */
  std::string getBindString();

  /**
   * @brief 获取获取函数名
   * @return 获取函数名字符串，如"getInt32"、"getString"等
   * @details 用于生成数据库获取函数名
   */
  std::string getGetString();

private:
  std::string m_name;      ///< 列名，用于标识列
  std::string m_type;      ///< 列类型字符串，如"int32_t"、"std::string"等
  std::string m_default;   ///< 默认值，根据类型不同存储不同格式
  std::string m_update;    ///< 更新值，用于指定列的更新规则
  std::string m_desc;      ///< 列描述，用于说明列的用途
  int m_index;             ///< 列索引，用于标识列在表中的位置

  bool m_autoIncrement;   ///< 是否为自增列，用于标识主键自增特性
  Type m_dtype;           ///< 数据类型枚举，用于类型判断和转换
  int m_length;           ///< 列长度，主要用于字符串类型
};

}   // namespace orm
}   // namespace sylar
