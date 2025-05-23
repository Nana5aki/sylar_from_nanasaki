/*
 * @Author: Nana5aki
 * @Date: 2025-05-17 20:58:44
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-05-23 00:42:57
 * @FilePath: /MySylar/sylar/db/sqlite3.h
 */
#pragma once

#include "db.h"
#include "sylar/mutex.h"
#include "sylar/singleton.h"
#include <cstdint>
#include <list>
#include <map>
#include <memory>
#include <sqlite3.h>

namespace sylar {
namespace DB {

/**
 * @brief SQLite3数据库连接类
 */
class SQLite3 : public IDB, public std::enable_shared_from_this<SQLite3> {
  friend class SQLite3Manager;

public:
  using ptr = std::shared_ptr<SQLite3>;
  enum Flags {
    READONLY = SQLITE_OPEN_READONLY,     ///< 只读模式
    READWRITE = SQLITE_OPEN_READWRITE,   ///< 读写模式
    CREATE = SQLITE_OPEN_CREATE          ///< 如果数据库不存在则创建
  };

public:
  /**
   * @brief 从已存在的sqlite3连接创建SQLite3对象
   * @param db sqlite3连接指针
   * @return SQLite3智能指针
   */
  static ptr Create(sqlite3* db);

  /**
   * @brief 创建新的SQLite3数据库连接
   * @param dbname 数据库文件名
   * @param flags 打开标志
   * @return SQLite3智能指针
   */
  static ptr Create(const std::string& dbname, int flags = READWRITE | CREATE);

public:
  ~SQLite3();

  IStmt::ptr prepare(const std::string& stmt) override;
  int getErrno() override;
  std::string getErrStr() override;
  int execute(const char* format, ...) override;
  int execute(const char* format, va_list ap);
  int execute(const std::string& sql) override;
  int64_t getLastInsertId() override;
  ISQLData::ptr query(const char* format, ...) override;
  ISQLData::ptr query(const std::string& sql) override;
  ITransaction::ptr openTransaction(bool auto_commit = false) override;

  /**
   * @brief 执行预处理语句
   * @tparam Args 参数类型
   * @param stmt SQL语句
   * @param args 参数列表
   * @return 执行结果
   */
  template <typename... Args>
  int execStmt(const char* stmt, Args&&... args);

  /**
   * @brief 查询预处理语句
   * @tparam Args 参数类型
   * @param stmt SQL语句
   * @param args 参数列表
   * @return 查询结果
   */
  template <class... Args>
  ISQLData::ptr queryStmt(const char* stmt, const Args&... args);

  /**
   * @brief 关闭数据库连接
   * @return 执行结果
   */
  int close();

  /**
   * @brief 获取原始sqlite3连接
   * @return sqlite3连接指针
   */
  sqlite3* getDB() const {
    return m_db;
  }

private:
  explicit SQLite3(sqlite3* db);

private:
  sqlite3* m_db;                 ///< sqlite3连接指针
  uint64_t m_lastUsedTime = 0;   ///< 最后使用时间
};

/**
 * @brief SQLite3预处理语句类
 */
class SQLite3Stmt : public IStmt, public std::enable_shared_from_this<SQLite3Stmt> {
  friend class SQLite3Data;

public:
  using ptr = std::shared_ptr<SQLite3Stmt>;
  enum Type { COPY = 1, REF = 2 };   ///< 绑定参数类型

public:
  /**
   * @brief 创建预处理语句
   * @param db 数据库连接
   * @param stmt SQL语句
   * @return 预处理语句智能指针
   */
  static SQLite3Stmt::ptr Create(SQLite3::ptr db, const char* stmt);

  /**
   * @brief 准备SQL语句
   * @param stmt SQL语句
   * @return 执行结果
   */
  int prepare(const char* stmt);

  ~SQLite3Stmt();

  /**
   * @brief 完成预处理语句
   * @return 执行结果
   */
  int finish();

  /**
   * @brief 绑定int32_t类型参数
   * @param idx 参数索引
   * @param value 参数值
   * @return 执行结果
   */
  int bind(int idx, int32_t value);

  /**
   * @brief 绑定uint32_t类型参数
   * @param idx 参数索引
   * @param value 参数值
   * @return 执行结果
   */
  int bind(int idx, uint32_t value);

  /**
   * @brief 绑定double类型参数
   * @param idx 参数索引
   * @param value 参数值
   * @return 执行结果
   */
  int bind(int idx, double value);

  /**
   * @brief 绑定int64_t类型参数
   * @param idx 参数索引
   * @param value 参数值
   * @return 执行结果
   */
  int bind(int idx, int64_t value);

  /**
   * @brief 绑定uint64_t类型参数
   * @param idx 参数索引
   * @param value 参数值
   * @return 执行结果
   */
  int bind(int idx, uint64_t value);

  /**
   * @brief 绑定字符串类型参数
   * @param idx 参数索引
   * @param value 参数值
   * @param type 绑定类型
   * @return 执行结果
   */
  int bind(int idx, const char* value, Type type = COPY);

  /**
   * @brief 绑定二进制数据参数
   * @param idx 参数索引
   * @param value 参数值
   * @param len 数据长度
   * @param type 绑定类型
   * @return 执行结果
   */
  int bind(int idx, const void* value, int len, Type type = COPY);

  /**
   * @brief 绑定字符串类型参数
   * @param idx 参数索引
   * @param value 参数值
   * @param type 绑定类型
   * @return 执行结果
   */
  int bind(int idx, const std::string& value, Type type = COPY);

  /**
   * @brief 绑定NULL类型参数
   * @param idx 参数索引
   * @return 执行结果
   */
  int bind(int idx);

  int bindInt8(int idx, const int8_t& value) override;
  int bindUint8(int idx, const uint8_t& value) override;
  int bindInt16(int idx, const int16_t& value) override;
  int bindUint16(int idx, const uint16_t& value) override;
  int bindInt32(int idx, const int32_t& value) override;
  int bindUint32(int idx, const uint32_t& value) override;
  int bindInt64(int idx, const int64_t& value) override;
  int bindUint64(int idx, const uint64_t& value) override;
  int bindFloat(int idx, const float& value) override;
  int bindDouble(int idx, const double& value) override;
  int bindString(int idx, const char* value) override;
  int bindString(int idx, const std::string& value) override;
  int bindBlob(int idx, const void* value, int64_t size) override;
  int bindBlob(int idx, const std::string& value) override;
  int bindTime(int idx, const time_t& value) override;
  int bindNull(int idx) override;

  int bind(const char* name, int32_t value);
  int bind(const char* name, uint32_t value);
  int bind(const char* name, double value);
  int bind(const char* name, int64_t value);
  int bind(const char* name, uint64_t value);
  int bind(const char* name, const char* value, Type type = COPY);
  int bind(const char* name, const void* value, int len, Type type = COPY);
  int bind(const char* name, const std::string& value, Type type = COPY);
  int bind(const char* name);

  /**
   * @brief 执行预处理语句
   * @return 执行结果
   */
  int step();

  /**
   * @brief 重置预处理语句
   * @return 执行结果
   */
  int reset();

  ISQLData::ptr query() override;
  int execute() override;
  int64_t getLastInsertId() override;
  int getErrno() override;
  std::string getErrStr() override;

protected:
  SQLite3Stmt(SQLite3::ptr db);

protected:
  SQLite3::ptr m_db;      ///< 数据库连接
  sqlite3_stmt* m_stmt;   ///< 预处理语句
};

/**
 * @brief SQLite3查询结果数据类
 */
class SQLite3Data : public ISQLData {
public:
  using ptr = std::shared_ptr<SQLite3Data>;

public:
  /**
   * @brief 构造函数
   * @param stmt 预处理语句
   * @param err 错误码
   * @param errstr 错误信息
   */
  SQLite3Data(std::shared_ptr<SQLite3Stmt> stmt, int err, const char* errstr);


  int getErrno() const override {
    return m_errno;
  }
  const std::string& getErrStr() const override {
    return m_errstr;
  }

  int getDataCount() override;
  int getColumnCount() override;
  int getColumnBytes(int idx) override;
  int getColumnType(int idx) override;
  std::string getColumnName(int idx) override;
  bool isNull(int idx) override;
  int8_t getInt8(int idx) override;
  uint8_t getUint8(int idx) override;
  int16_t getInt16(int idx) override;
  uint16_t getUint16(int idx) override;
  int32_t getInt32(int idx) override;
  uint32_t getUint32(int idx) override;
  int64_t getInt64(int idx) override;
  uint64_t getUint64(int idx) override;
  float getFloat(int idx) override;
  double getDouble(int idx) override;
  std::string getString(int idx) override;
  std::string getBlob(int idx) override;
  time_t getTime(int idx) override;
  bool next() override;

private:
  int m_errno;               ///< 错误码
  bool m_first;              ///< 是否为第一行
  std::string m_errstr;      ///< 错误信息
  SQLite3Stmt::ptr m_stmt;   ///< 预处理语句
};

/**
 * @brief SQLite3事务类
 */
class SQLite3Transaction : public ITransaction {
public:
  enum Type {
    DEFERRED = 0,    ///< 延迟事务
    IMMEDIATE = 1,   ///< 立即事务
    EXCLUSIVE = 2    ///< 排他事务
  };

public:
  /**
   * @brief 创建事务对象
   * @param db 数据库连接
   * @param auto_commit 是否自动提交
   * @param type 事务类型
   * @return 事务对象智能指针
   */
  static ptr Create(SQLite3::ptr db, bool auto_commit = false, Type type = DEFERRED);

public:
  /**
   * @brief 构造函数
   * @param db 数据库连接
   * @param auto_commit 是否自动提交
   * @param type 事务类型
   */
  SQLite3Transaction(SQLite3::ptr db, bool auto_commit = false, Type type = DEFERRED);
  ~SQLite3Transaction();
  bool begin() override;
  bool commit() override;
  bool rollback() override;
  int execute(const char* format, ...) override;
  int execute(const std::string& sql) override;
  int64_t getLastInsertId() override;

private:
  SQLite3::ptr m_db;   ///< 数据库连接
  Type m_type;         ///< 事务类型
  int8_t m_status;     ///< 事务状态
  bool m_autoCommit;   ///< 是否自动提交
};

/**
 * @brief SQLite3连接管理器类
 */
class SQLite3Manager {
public:
  using MutexType = sylar::Mutex;

public:
  SQLite3Manager();
  ~SQLite3Manager();

  /**
   * @brief 获取数据库连接
   * @param name 数据库名称
   * @return 数据库连接智能指针
   */
  SQLite3::ptr get(const std::string& name);

  /**
   * @brief 注册数据库连接
   * @param name 数据库名称
   * @param params 连接参数
   */
  void registerSQLite3(const std::string& name, const std::map<std::string, std::string>& params);

  /**
   * @brief 检查连接状态
   * @param sec 超时时间(秒)
   */
  void checkConnection(int sec = 30);

  /**
   * @brief 获取最大连接数
   * @return 最大连接数
   */
  uint32_t getMaxConn() const {
    return m_maxConn;
  }

  /**
   * @brief 设置最大连接数
   * @param v 最大连接数
   */
  void setMaxConn(uint32_t v) {
    m_maxConn = v;
  }

  /**
   * @brief 执行SQL语句(格式化字符串版本)
   * @param name 数据库名称
   * @param format 格式化字符串
   * @param ... 可变参数
   * @return 执行结果
   */
  int execute(const std::string& name, const char* format, ...);

  /**
   * @brief 执行SQL语句(va_list版本)
   * @param name 数据库名称
   * @param format 格式化字符串
   * @param ap 参数列表
   * @return 执行结果
   */
  int execute(const std::string& name, const char* format, va_list ap);

  /**
   * @brief 执行SQL语句(字符串版本)
   * @param name 数据库名称
   * @param sql SQL语句
   * @return 执行结果
   */
  int execute(const std::string& name, const std::string& sql);

  /**
   * @brief 查询SQL语句(格式化字符串版本)
   * @param name 数据库名称
   * @param format 格式化字符串
   * @param ... 可变参数
   * @return 查询结果
   */
  ISQLData::ptr query(const std::string& name, const char* format, ...);

  /**
   * @brief 查询SQL语句(va_list版本)
   * @param name 数据库名称
   * @param format 格式化字符串
   * @param ap 参数列表
   * @return 查询结果
   */
  ISQLData::ptr query(const std::string& name, const char* format, va_list ap);

  /**
   * @brief 查询SQL语句(字符串版本)
   * @param name 数据库名称
   * @param sql SQL语句
   * @return 查询结果
   */
  ISQLData::ptr query(const std::string& name, const std::string& sql);

  /**
   * @brief 开启事务
   * @param name 数据库名称
   * @param auto_commit 是否自动提交
   * @return 事务对象智能指针
   */
  SQLite3Transaction::ptr openTransaction(const std::string& name, bool auto_commit);

private:
  /**
   * @brief 释放数据库连接
   * @param name 数据库名称
   * @param m 数据库连接指针
   */
  void freeSQLite3(const std::string& name, SQLite3* m);

private:
  uint32_t m_maxConn;                                                      ///< 最大连接数
  MutexType m_mutex;                                                       ///< 互斥锁
  std::map<std::string, std::list<SQLite3*>> m_conns;                      ///< 连接池
  std::map<std::string, std::map<std::string, std::string>> m_dbDefines;   ///< 数据库定义
};

using SQLite3Mgr = sylar::Singleton<SQLite3Manager>;

namespace {

template <size_t N, typename... Args>
struct SQLite3Binder {
  static int Bind(std::shared_ptr<SQLite3Stmt> stmt) {
    return SQLITE_OK;
  }
};

/**
 * @brief 辅助函数，用于绑定参数到SQL语句
 * @tparam Args 可变参数类型
 * @param stmt SQL语句对象
 * @param args 要绑定的参数
 * @return SQLite操作结果码
 */
template <typename... Args>
int bindX(SQLite3Stmt::ptr stmt, const Args&... args) {
  return SQLite3Binder<1, Args...>::Bind(stmt, args...);
}
}   // namespace

template <typename... Args>
int SQLite3::execStmt(const char* stmt, Args&&... args) {
  auto st = SQLite3Stmt::Create(shared_from_this(), stmt);
  if (!st) {
    return -1;
  }
  int rt = bindX(st, args...);
  if (rt != SQLITE_OK) {
    return rt;
  }
  return st->execute();
}

template <class... Args>
ISQLData::ptr SQLite3::queryStmt(const char* stmt, const Args&... args) {
  auto st = SQLite3Stmt::Create(shared_from_this(), stmt);
  if (!st) {
    return nullptr;
  }
  int rt = bindX(st, args...);
  if (rt != SQLITE_OK) {
    return nullptr;
  }
  return st->query();
}

namespace {

/**
 * @brief SQLite3Binder的主模板声明
 * @tparam N 参数绑定的索引位置
 * @tparam Args 可变参数模板，用于处理不同类型的参数
 */
template <size_t N, typename... Args>
struct SQLite3Binder;

/**
 * @brief 处理字符串字面量的特化版本
 * @tparam N 参数绑定的索引位置
 * @tparam Tail 剩余参数类型
 */
template <size_t N, typename... Tail>
struct SQLite3Binder<N, const char (&)[], Tail...> {
  /**
   * @brief 将字符串字面量绑定到SQL语句
   * @param stmt SQL语句对象
   * @param value 字符串字面量
   * @param tail 剩余参数
   * @return SQLite操作结果码
   */
  static int Bind(SQLite3Stmt::ptr stmt, const char value[], const Tail&... tail) {
    int rt = stmt->bind(N, (const char*)value);
    if (rt != SQLITE_OK) {
      return rt;
    }
    return SQLite3Binder<N + 1, Tail...>::Bind(stmt, tail...);
  }
};

/**
 * @brief 默认特化版本，用于处理不支持的类型
 * @tparam N 参数绑定的索引位置
 * @tparam Head 当前参数类型
 * @tparam Tail 剩余参数类型
 */
template <size_t N, typename Head, typename... Tail>
struct SQLite3Binder<N, Head, Tail...> {
  /**
   * @brief 对不支持的类型在编译时产生错误
   * @param stmt SQL语句对象
   * @param head 当前参数
   * @param tail 剩余参数
   * @return SQLite操作结果码
   */
  static int Bind(SQLite3Stmt::ptr stmt, const Head&, const Tail&...) {
    static_assert(sizeof...(Tail) < 0, "invalid type");
    return SQLITE_OK;
  }
};

/**
 * @brief 用于生成基本类型特化的宏
 * @param type 原始类型
 * @param type2 绑定到SQLite时使用的类型
 */
#define XX(type, type2)                                                               \
  template <size_t N, typename... Tail>                                               \
  struct SQLite3Binder<N, type, Tail...> {                                            \
    static int Bind(SQLite3Stmt::ptr stmt, const type2& value, const Tail&... tail) { \
      int rt = stmt->bind(N, value);                                                  \
      if (rt != SQLITE_OK) {                                                          \
        return rt;                                                                    \
      }                                                                               \
      return SQLite3Binder<N + 1, Tail...>::Bind(stmt, tail...);                      \
    }                                                                                 \
  };

// 基本类型特化定义
XX(char*, char* const);         // 字符指针
XX(const char*, char* const);   // 常量字符指针
XX(std::string, std::string);   // 字符串
XX(int8_t, int32_t);            // 8位整数
XX(uint8_t, int32_t);           // 8位无符号整数
XX(int16_t, int32_t);           // 16位整数
XX(uint16_t, int32_t);          // 16位无符号整数
XX(int32_t, int32_t);           // 32位整数
XX(uint32_t, int32_t);          // 32位无符号整数
XX(int64_t, int64_t);           // 64位整数
XX(uint64_t, int64_t);          // 64位无符号整数
XX(float, double);              // 浮点数
XX(double, double);             // 双精度浮点数
#undef XX
}   // namespace

}   // namespace DB
}   // namespace sylar