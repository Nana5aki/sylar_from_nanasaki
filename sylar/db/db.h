/*
 * @Author: Nana5aki
 * @Date: 2025-05-13 21:31:33
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-05-17 20:50:18
 * @FilePath: /sylar_from_nanasaki/sylar/db/db.h
 */
#pragma once

#include <memory>
#include <string>

namespace sylar {
namespace DB {

/**
 * @brief SQL查询结果数据接口
 * @details 用于封装数据库查询返回的结果集
 */
class ISQLData {
public:
  using ptr = std::shared_ptr<ISQLData>;

  virtual ~ISQLData() = default;

  /**
   * @brief 获取错误码
   */
  virtual int getErrno() const = 0;

  /**
   * @brief 获取错误信息
   */
  virtual const std::string& getErrStr() const = 0;

  /**
   * @brief 获取结果集行数
   */
  virtual int getDataCount() = 0;

  /**
   * @brief 获取结果集列数
   */
  virtual int getColumnCount() = 0;

  /**
   * @brief 获取指定列的数据大小(字节)
   */
  virtual int getColumnBytes(int idx) = 0;

  /**
   * @brief 获取指定列的数据类型
   */
  virtual int getColumnType(int idx) = 0;

  /**
   * @brief 获取指定列的列名
   */
  virtual std::string getColumnName(int idx) = 0;

  /**
   * @brief 判断指定列的值是否为NULL
   */
  virtual bool isNull(int idx) = 0;

  // 获取各种数据类型的值
  virtual int8_t getInt8(int idx) = 0;
  virtual uint8_t getUint8(int idx) = 0;
  virtual int16_t getInt16(int idx) = 0;
  virtual uint16_t getUint16(int idx) = 0;
  virtual int32_t getInt32(int idx) = 0;
  virtual uint32_t getUint32(int idx) = 0;
  virtual int64_t getInt64(int idx) = 0;
  virtual uint64_t getUint64(int idx) = 0;
  virtual float getFloat(int idx) = 0;
  virtual double getDouble(int idx) = 0;
  virtual std::string getString(int idx) = 0;
  virtual std::string getBlob(int idx) = 0;
  virtual time_t getTime(int idx) = 0;

  /**
   * @brief 移动到结果集的下一行
   * @return 是否还有下一行数据
   */
  virtual bool next() = 0;
};

/**
 * @brief SQL更新操作接口
 * @details 用于执行INSERT、UPDATE、DELETE等更新操作
 */
class ISQLUpdate {
public:
  virtual ~ISQLUpdate() = default;

  /**
   * @brief 执行SQL更新语句(格式化字符串版本)
   */
  virtual int execute(const char* format, ...) = 0;

  /**
   * @brief 执行SQL更新语句(string版本)
   */
  virtual int execute(const std::string& sql) = 0;

  /**
   * @brief 获取最后插入记录的ID
   */
  virtual int64_t getLastInsertId() = 0;
};

/**
 * @brief SQL查询操作接口
 * @details 用于执行SELECT等查询操作
 */
class ISQLQuery {
public:
  virtual ~ISQLQuery() = default;

  /**
   * @brief 执行SQL查询语句(格式化字符串版本)
   */
  virtual ISQLData::ptr query(const char* format, ...) = 0;

  /**
   * @brief 执行SQL查询语句(string版本)
   */
  virtual ISQLData::ptr query(const std::string& sql) = 0;
};

/**
 * @brief SQL预处理语句接口
 * @details 用于执行预编译的SQL语句，防止SQL注入
 */
class IStmt {
public:
  using ptr = std::shared_ptr<IStmt>;

  virtual ~IStmt() = default;

  // 绑定各种数据类型的参数
  virtual int bindInt8(int idx, const int8_t& value) = 0;
  virtual int bindUint8(int idx, const uint8_t& value) = 0;
  virtual int bindInt16(int idx, const int16_t& value) = 0;
  virtual int bindUint16(int idx, const uint16_t& value) = 0;
  virtual int bindInt32(int idx, const int32_t& value) = 0;
  virtual int bindUint32(int idx, const uint32_t& value) = 0;
  virtual int bindInt64(int idx, const int64_t& value) = 0;
  virtual int bindUint64(int idx, const uint64_t& value) = 0;
  virtual int bindFloat(int idx, const float& value) = 0;
  virtual int bindDouble(int idx, const double& value) = 0;
  virtual int bindString(int idx, const char* value) = 0;
  virtual int bindString(int idx, const std::string& value) = 0;
  virtual int bindBlob(int idx, const void* value, int64_t size) = 0;
  virtual int bindBlob(int idx, const std::string& value) = 0;
  virtual int bindTime(int idx, const time_t& value) = 0;
  virtual int bindNull(int idx) = 0;

  /**
   * @brief 执行预处理语句
   */
  virtual int execute() = 0;

  /**
   * @brief 获取最后插入记录的ID
   */
  virtual int64_t getLastInsertId() = 0;

  /**
   * @brief 执行查询并返回结果集
   */
  virtual ISQLData::ptr query() = 0;

  /**
   * @brief 获取错误码
   */
  virtual int getErrno() = 0;

  /**
   * @brief 获取错误信息
   */
  virtual std::string getErrStr() = 0;
};

/**
 * @brief 数据库事务接口
 * @details 用于管理数据库事务，支持事务的开启、提交和回滚
 */
class ITransaction : public ISQLUpdate {
public:
  using ptr = std::shared_ptr<ITransaction>;
  virtual ~ITransaction() = default;

  /**
   * @brief 开启事务
   */
  virtual bool begin() = 0;

  /**
   * @brief 提交事务
   */
  virtual bool commit() = 0;

  /**
   * @brief 回滚事务
   */
  virtual bool rollback() = 0;
};

/**
 * @brief 数据库连接接口
 * @details 提供数据库连接的基本操作，包括查询、更新和事务管理
 */
class IDB : public ISQLUpdate, public ISQLQuery {
public:
  using ptr = std::shared_ptr<IDB>;
  virtual ~IDB() = default;

  /**
   * @brief 准备预处理语句
   */
  virtual IStmt::ptr prepare(const std::string& stmt) = 0;

  /**
   * @brief 获取错误码
   */
  virtual int getErrno() = 0;

  /**
   * @brief 获取错误信息
   */
  virtual std::string getErrStr() = 0;

  /**
   * @brief 开启事务
   * @param auto_commit 是否自动提交
   */
  virtual ITransaction::ptr openTransaction(bool auto_commit = false) = 0;
};

}   // namespace DB
}   // namespace sylar