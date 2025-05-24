/*
 * @Author: Nana5aki
 * @Date: 2025-05-08 23:55:59
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-05-24 16:56:10
 * @FilePath: /sylar_from_nanasaki/sylar/orm/table.h
 * @Description: ORM表定义类，用于生成数据库表相关的代码
 */
#pragma once

#include "column.h"
#include "index.h"
#include <memory>
#include <string>
#include <vector>

namespace sylar {
namespace orm {

/**
 * @brief 数据库表定义类
 * @details 用于定义数据库表的结构，包括表名、命名空间、列定义、索引等信息
 *          并提供代码生成功能
 */
class Table {
public:
  using ptr = std::shared_ptr<Table>;

  /**
   * @brief 获取表名
   * @return 表名字符串
   */
  const std::string& getName() const {
    return m_name;
  }

  /**
   * @brief 获取命名空间
   * @return 命名空间字符串
   */
  const std::string& getNamespace() const {
    return m_namespace;
  }

  /**
   * @brief 获取表描述
   * @return 表描述字符串
   */
  const std::string& getDesc() const {
    return m_desc;
  }

  /**
   * @brief 获取所有列定义
   * @return 列定义向量
   */
  const std::vector<Column::ptr>& getCols() const {
    return m_cols;
  }

  /**
   * @brief 获取所有索引定义
   * @return 索引定义向量
   */
  const std::vector<Index::ptr>& getIdxs() const {
    return m_idxs;
  }

  /**
   * @brief 从XML节点初始化表定义
   * @param node XML节点
   * @return 是否初始化成功
   */
  bool init(const tinyxml2::XMLElement& node);

  /**
   * @brief 生成代码
   * @param path 代码生成路径
   */
  void gen(const std::string& path);

  /**
   * @brief 获取生成的文件名
   * @return 文件名
   */
  std::string getFilename() const;

private:
  /**
   * @brief 生成头文件
   * @param path 生成路径
   */
  void gen_inc(const std::string& path);

  /**
   * @brief 生成源文件
   * @param path 生成路径
   */
  void gen_src(const std::string& path);

  /**
   * @brief 生成toString函数声明
   * @return 函数声明字符串
   */
  std::string genToStringInc();

  /**
   * @brief 生成toString函数实现
   * @param class_name 类名
   * @return 函数实现字符串
   */
  std::string genToStringSrc(const std::string& class_name);

  /**
   * @brief 生成插入SQL语句
   * @param class_name 类名
   * @return SQL语句字符串
   */
  std::string genToInsertSQL(const std::string& class_name);

  /**
   * @brief 生成更新SQL语句
   * @param class_name 类名
   * @return SQL语句字符串
   */
  std::string genToUpdateSQL(const std::string& class_name);

  /**
   * @brief 生成删除SQL语句
   * @param class_name 类名
   * @return SQL语句字符串
   */
  std::string genToDeleteSQL(const std::string& class_name);

  /**
   * @brief 获取主键列
   * @return 主键列向量
   */
  std::vector<Column::ptr> getPKs() const;

  /**
   * @brief 根据列名获取列定义
   * @param name 列名
   * @return 列定义指针
   */
  Column::ptr getCol(const std::string& name) const;

  /**
   * @brief 生成WHERE子句
   * @return WHERE子句字符串
   */
  std::string genWhere() const;

  /**
   * @brief 生成DAO头文件
   * @param ofs 输出流
   */
  void gen_dao_inc(std::ofstream& ofs);

  /**
   * @brief 生成DAO源文件
   * @param ofs 输出流
   */
  void gen_dao_src(std::ofstream& ofs);


  bool validateRequiredAttributes(const tinyxml2::XMLElement& node);
  bool initColumns(const tinyxml2::XMLElement& node);
  bool initIndexes(const tinyxml2::XMLElement& node);
  bool validateIndexColumns(const Index::ptr& idx);
  void genIncludeFiles(std::ofstream& ofs);
  void genNamespaces(std::ofstream& ofs, bool open);
  void genClassDefinition(std::ofstream& ofs, const std::string& class_name,
                          const std::string& class_name_dao);
  void genGettersAndSetters(std::ofstream& ofs);
  void genPrivateMembers(std::ofstream& ofs);
  void genConstructor(std::ofstream& ofs, const std::string& class_name);
  void genSetterImplementations(std::ofstream& ofs, const std::string& class_name);
  void genBasicCRUD(std::ofstream& ofs, const std::string& class_name);
  void genIndexOperations(std::ofstream& ofs, const std::string& class_name);
  void genIndexDeleteOperation(std::ofstream& ofs, const std::string& class_name,
                               const Index::ptr& idx);
  void genUniqueIndexQueryOperation(std::ofstream& ofs, const std::string& class_name,
                                    const Index::ptr& idx);
  void genIndexQueryOperation(std::ofstream& ofs, const std::string& class_name,
                              const Index::ptr& idx);
  void genCreateTableOperations(std::ofstream& ofs);
  void genUpdateImpl(std::ofstream& ofs, const std::string& class_name,
                     const std::string& class_name_dao);
  void genInsertImpl(std::ofstream& ofs, const std::string& class_name,
                     const std::string& class_name_dao);
  void genPrepareStatement(std::ofstream& ofs, const std::string& error_return);
  void genInsertOrUpdateImpl(std::ofstream& ofs, const std::string& class_name,
                             const std::string& class_name_dao);
  void genDeleteImpl(std::ofstream& ofs, const std::string& class_name,
                     const std::string& class_name_dao);
  void genIndexDeleteImpl(std::ofstream& ofs, const std::string& class_name,
                          const std::string& class_name_dao);
  void genQueryAllImpl(std::ofstream& ofs, const std::string& class_name,
                       const std::string& class_name_dao);
  void genQueryImpl(std::ofstream& ofs, const std::string& class_name,
                    const std::string& class_name_dao);
  void genIndexQueryImpl(std::ofstream& ofs, const std::string& class_name,
                         const std::string& class_name_dao);
  void genCreateTableImpl(std::ofstream& ofs, const std::string& class_name_dao);

private:
  /**
   * @brief 数据库类型枚举
   */
  enum DBType { TYPE_SQLITE3 = 1, TYPE_MYSQL = 2 };

private:
  std::string m_name;                             ///< 表名
  std::string m_namespace;                        ///< 命名空间
  std::string m_desc;                             ///< 表描述
  std::string m_subfix = "_info";                 ///< 类名后缀
  DBType m_type = TYPE_SQLITE3;                   ///< 数据库类型
  std::string m_dbclass = "sylar::DB::IDB";           ///< 数据库类名
  std::string m_queryclass = "sylar::DB::IDB";    ///< 查询类名
  std::string m_updateclass = "sylar::DB::IDB";   ///< 更新类名
  std::vector<Column::ptr> m_cols;                ///< 列定义
  std::vector<Index::ptr> m_idxs;                 ///< 索引定义
};

}   // namespace orm
}   // namespace sylar