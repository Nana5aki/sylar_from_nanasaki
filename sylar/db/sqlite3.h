/*
 * @Author: Nana5aki
 * @Date: 2025-05-17 20:58:44
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-05-18 18:02:53
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

class SQLite3 : public IDB, public std::enable_shared_from_this<SQLite3> {
  friend class SQLite3Manager;

public:
  using ptr = std::shared_ptr<SQLite3>;
  enum Flags {
    READONLY = SQLITE_OPEN_READONLY,
    READWRITE = SQLITE_OPEN_READWRITE,
    CREATE = SQLITE_OPEN_CREATE
  };

public:
  static ptr Create(sqlite3* db);
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

  template <typename... Args>
  int execStmt(const char* stmt, Args&&... args);

  template <class... Args>
  ISQLData::ptr queryStmt(const char* stmt, const Args&... args);

  int close();

  sqlite3* getDB() const {
    return m_db;
  }

private:
  explicit SQLite3(sqlite3* db);

private:
  sqlite3* m_db;
  uint64_t m_lastUsedTime = 0;
};

class SQLite3Stmt : public IStmt, public std::enable_shared_from_this<SQLite3Stmt> {
  friend class SQLite3Data;

public:
  using ptr = std::shared_ptr<SQLite3Stmt>;
  enum Type { COPY = 1, REF = 2 };

public:
  static SQLite3Stmt::ptr Create(SQLite3::ptr db, const char* stmt);

  int prepare(const char* stmt);
  ~SQLite3Stmt();
  int finish();

  int bind(int idx, int32_t value);
  int bind(int idx, uint32_t value);
  int bind(int idx, double value);
  int bind(int idx, int64_t value);
  int bind(int idx, uint64_t value);
  int bind(int idx, const char* value, Type type = COPY);
  int bind(int idx, const void* value, int len, Type type = COPY);
  int bind(int idx, const std::string& value, Type type = COPY);
  // for null type
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
  // for null type
  int bind(const char* name);

  int step();
  int reset();

  ISQLData::ptr query() override;
  int execute() override;
  int64_t getLastInsertId() override;

  int getErrno() override;
  std::string getErrStr() override;

protected:
  SQLite3Stmt(SQLite3::ptr db);

protected:
  SQLite3::ptr m_db;
  sqlite3_stmt* m_stmt;
};

class SQLite3Data : public ISQLData {
public:
  using ptr = std::shared_ptr<SQLite3Data>;

public:
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
  int m_errno;
  bool m_first;
  std::string m_errstr;
  SQLite3Stmt::ptr m_stmt;
};

class SQLite3Transaction : public ITransaction {
public:
  enum Type { DEFERRED = 0, IMMEDIATE = 1, EXCLUSIVE = 2 };

public:
  static ptr Create(SQLite3::ptr db, bool auto_commit = false, Type type = DEFERRED);

public:
  SQLite3Transaction(SQLite3::ptr db, bool auto_commit = false, Type type = DEFERRED);
  ~SQLite3Transaction();
  bool begin() override;
  bool commit() override;
  bool rollback() override;
  int execute(const char* format, ...) override;
  int execute(const std::string& sql) override;
  int64_t getLastInsertId() override;

private:
  SQLite3::ptr m_db;
  Type m_type;
  int8_t m_status;
  bool m_autoCommit;
};

class SQLite3Manager {
public:
  using MutexType = sylar::Mutex;

public:
  SQLite3Manager();
  ~SQLite3Manager();

  SQLite3::ptr get(const std::string& name);
  void registerSQLite3(const std::string& name, const std::map<std::string, std::string>& params);

  void checkConnection(int sec = 30);

  uint32_t getMaxConn() const {
    return m_maxConn;
  }
  void setMaxConn(uint32_t v) {
    m_maxConn = v;
  }

  int execute(const std::string& name, const char* format, ...);
  int execute(const std::string& name, const char* format, va_list ap);
  int execute(const std::string& name, const std::string& sql);

  ISQLData::ptr query(const std::string& name, const char* format, ...);
  ISQLData::ptr query(const std::string& name, const char* format, va_list ap);
  ISQLData::ptr query(const std::string& name, const std::string& sql);

  SQLite3Transaction::ptr openTransaction(const std::string& name, bool auto_commit);

private:
  void freeSQLite3(const std::string& name, SQLite3* m);

private:
  uint32_t m_maxConn;
  MutexType m_mutex;
  std::map<std::string, std::list<SQLite3*>> m_conns;
  std::map<std::string, std::map<std::string, std::string>> m_dbDefines;
};

using SQLite3Mgr = sylar::Singleton<SQLite3Manager>;

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