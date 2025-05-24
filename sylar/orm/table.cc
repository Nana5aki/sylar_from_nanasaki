/*
 * @Author: Nana5aki
 * @Date: 2025-05-08 23:56:10
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-05-24 17:44:25
 * @FilePath: /sylar_from_nanasaki/sylar/orm/table.cc
 * @Description: ORM表实现类，用于生成数据库表相关的代码
 */
#include "table.h"
#include "orm_utils.h"
#include "sylar/log.h"
#include "sylar/util/fs_util.h"
#include "sylar/util/string_util.h"
#include <algorithm>
#include <fstream>
#include <set>
#include <sstream>

namespace sylar {
namespace orm {

static Logger::ptr g_logger = SYLAR_LOG_NAME("orm");

// 常量定义
constexpr const char* COLUMN_TAG = "column";
constexpr const char* COLUMNS_TAG = "columns";
constexpr const char* INDEX_TAG = "index";
constexpr const char* INDEXS_TAG = "indexs";

bool Table::init(const tinyxml2::XMLElement& node) {
  // 验证必要的属性
  if (!validateRequiredAttributes(node)) {
    return false;
  }

  // 初始化列定义
  if (!initColumns(node)) {
    return false;
  }

  // 初始化索引定义
  if (!initIndexes(node)) {
    return false;
  }

  return true;
}

bool Table::validateRequiredAttributes(const tinyxml2::XMLElement& node) {
  if (!node.Attribute("name")) {
    SYLAR_LOG_ERROR(g_logger) << "table name is null";
    return false;
  }
  m_name = node.Attribute("name");

  if (!node.Attribute("namespace")) {
    SYLAR_LOG_ERROR(g_logger) << "table namespace is null";
    return false;
  }
  m_namespace = node.Attribute("namespace");

  if (node.Attribute("desc")) {
    m_desc = node.Attribute("desc");
  }
  return true;
}

bool Table::initColumns(const tinyxml2::XMLElement& node) {
  const tinyxml2::XMLElement* cols = node.FirstChildElement(COLUMNS_TAG);
  if (!cols) {
    SYLAR_LOG_ERROR(g_logger) << "table name=" << m_name << " columns is null";
    return false;
  }

  const tinyxml2::XMLElement* col = cols->FirstChildElement(COLUMN_TAG);
  if (!col) {
    SYLAR_LOG_ERROR(g_logger) << "table name=" << m_name << " column is null";
    return false;
  }

  std::set<std::string> col_names;
  int index = 0;

  do {
    Column::ptr col_ptr = std::make_shared<Column>();
    if (!col_ptr->init(*col)) {
      SYLAR_LOG_ERROR(g_logger) << "table name=" << m_name << " init column error";
      return false;
    }

    const std::string& col_name = col_ptr->getName();
    if (!col_names.insert(col_name).second) {
      SYLAR_LOG_ERROR(g_logger) << "table name=" << m_name << " column name=" << col_name
                                << " exists";
      return false;
    }

    col_ptr->m_index = index++;
    m_cols.push_back(col_ptr);
    col = col->NextSiblingElement(COLUMN_TAG);
  } while (col);

  return true;
}

bool Table::initIndexes(const tinyxml2::XMLElement& node) {
  const tinyxml2::XMLElement* idxs = node.FirstChildElement(INDEXS_TAG);
  if (!idxs) {
    SYLAR_LOG_ERROR(g_logger) << "table name=" << m_name << " indexs is null";
    return false;
  }

  const tinyxml2::XMLElement* idx = idxs->FirstChildElement(INDEX_TAG);
  if (!idx) {
    SYLAR_LOG_ERROR(g_logger) << "table name=" << m_name << " index is null";
    return false;
  }

  std::set<std::string> idx_names;
  bool has_pk = false;

  do {
    Index::ptr idx_ptr = std::make_shared<Index>();
    if (!idx_ptr->init(*idx)) {
      SYLAR_LOG_ERROR(g_logger) << "table name=" << m_name << " index init error";
      return false;
    }

    const std::string& idx_name = idx_ptr->getName();
    if (!idx_names.insert(idx_name).second) {
      SYLAR_LOG_ERROR(g_logger) << "table name=" << m_name << " index name=" << idx_name
                                << " exists";
      return false;
    }

    if (idx_ptr->isPK()) {
      if (has_pk) {
        SYLAR_LOG_ERROR(g_logger) << "table name=" << m_name << " more than one pk";
        return false;
      }
      has_pk = true;
    }

    // 验证索引列是否存在
    if (!validateIndexColumns(idx_ptr)) {
      return false;
    }

    m_idxs.push_back(idx_ptr);
    idx = idx->NextSiblingElement(INDEX_TAG);
  } while (idx);

  return true;
}

bool Table::validateIndexColumns(const Index::ptr& idx) {
  std::set<std::string> col_names;
  for (const auto& col : m_cols) {
    col_names.insert(col->getName());
  }

  for (const auto& col_name : idx->getCols()) {
    if (col_names.count(col_name) == 0) {
      SYLAR_LOG_ERROR(g_logger) << "table name=" << m_name << " idx=" << idx->getName()
                                << " col=" << col_name << " not exists";
      return false;
    }
  }
  return true;
}

std::string Table::getFilename() const {
  return sylar::string_util::ToLower(m_name + m_subfix);
}

void Table::gen(const std::string& path) {
  try {
    std::string p = path + "/" + sylar::string_util::replace(m_namespace, ".", "/");
    if (!sylar::fs_util::Mkdir(p)) {
      SYLAR_LOG_ERROR(g_logger) << "Failed to create directory: " << p;
      return;
    }
    gen_inc(p);
    gen_src(p);
  } catch (const std::exception& e) {
    SYLAR_LOG_ERROR(g_logger) << "Failed to generate code: " << e.what();
  }
}

void Table::gen_inc(const std::string& path) {
  std::string filename = path + "/" + m_name + m_subfix + ".h";
  std::string class_name = m_name + m_subfix;
  std::string class_name_dao = m_name + m_subfix + "_dao";

  try {
    std::ofstream ofs(filename);
    if (!ofs.is_open()) {
      SYLAR_LOG_ERROR(g_logger) << "Failed to open file: " << filename;
      return;
    }

    ofs << "// clang-format off" << std::endl;
    // 生成头文件保护宏
    // std::string macro_name = GetAsDefineMacro(m_namespace + class_name + ".h");
    // ofs << "#ifndef " << macro_name << std::endl;
    // ofs << "#define " << macro_name << std::endl << std::endl;
    ofs << "#pragma once" << std::endl;

    // 生成包含文件
    genIncludeFiles(ofs);

    // 生成命名空间
    genNamespaces(ofs, true);

    // 生成类定义
    genClassDefinition(ofs, class_name, class_name_dao);

    // 生成DAO类定义
    gen_dao_inc(ofs);

    // 关闭命名空间
    genNamespaces(ofs, false);

    ofs << "// clang-format on" << std::endl;

    // 结束头文件保护宏
    // ofs << "#endif //" << macro_name << std::endl;
  } catch (const std::exception& e) {
    SYLAR_LOG_ERROR(g_logger) << "Failed to generate header file: " << e.what();
  }
}

void Table::genIncludeFiles(std::ofstream& ofs) {
  // 系统头文件
  const std::set<std::string> system_includes = {"vector", "string", "memory"};
  for (const auto& inc : system_includes) {
    ofs << "#include <" << inc << ">" << std::endl;
  }
  ofs << std::endl;

  // 项目头文件
  const std::set<std::string> project_includes = {"sylar/db/db.h"};
  for (const auto& inc : project_includes) {
    ofs << "#include \"" << inc << "\"" << std::endl;
  }
  ofs << std::endl << std::endl;
}

void Table::genNamespaces(std::ofstream& ofs, bool open) {
  std::vector<std::string> ns = sylar::string_util::split(m_namespace, '.');
  if (open) {
    for (const auto& n : ns) {
      ofs << "namespace " << n << " {" << std::endl;
    }
    ofs << std::endl;
  } else {
    for (auto it = ns.rbegin(); it != ns.rend(); ++it) {
      ofs << "} //namespace " << *it << std::endl;
    }
  }
}

void Table::genClassDefinition(std::ofstream& ofs, const std::string& class_name,
                               const std::string& class_name_dao) {
  ofs << "class " << GetAsClassName(class_name_dao) << ";" << std::endl;
  ofs << "class " << GetAsClassName(class_name) << " {" << std::endl;
  ofs << "friend class " << GetAsClassName(class_name_dao) << ";" << std::endl;
  ofs << "public:" << std::endl;
  ofs << "  using ptr = std::shared_ptr<" << GetAsClassName(class_name) << ">;" << std::endl;
  ofs << std::endl;
  ofs << "  " << GetAsClassName(class_name) << "();" << std::endl;
  ofs << std::endl;

  // 生成getter和setter
  genGettersAndSetters(ofs);

  ofs << "  " << genToStringInc() << std::endl;
  ofs << std::endl;

  // 生成私有成员
  genPrivateMembers(ofs);

  ofs << "};" << std::endl;
  ofs << std::endl;
}

void Table::genGettersAndSetters(std::ofstream& ofs) {
  for (const auto& col : m_cols) {
    ofs << "  " << col->getGetFunDefine();
    ofs << "  " << col->getSetFunDefine();
    ofs << std::endl;
  }
}

void Table::genPrivateMembers(std::ofstream& ofs) {
  ofs << "private:" << std::endl;
  auto cols = m_cols;
  std::sort(cols.begin(), cols.end(), [](const Column::ptr& a, const Column::ptr& b) {
    if (a->getDType() != b->getDType()) {
      return a->getDType() < b->getDType();
    }
    return a->getIndex() < b->getIndex();
  });

  for (const auto& col : cols) {
    ofs << "  " << col->getMemberDefine();
  }
}

void Table::gen_src(const std::string& path) {
  std::string class_name = m_name + m_subfix;
  std::string filename = path + "/" + class_name + ".cc";

  try {
    std::ofstream ofs(filename);
    if (!ofs.is_open()) {
      SYLAR_LOG_ERROR(g_logger) << "Failed to open file: " << filename;
      return;
    }

    ofs << "// clang-format off" << std::endl;
    // 生成包含文件
    ofs << "#include \"" << class_name + ".h\"" << std::endl;
    ofs << "#include \"sylar/log.h\"" << std::endl;
    ofs << "#include \"sylar/util/util.h\"" << std::endl;
    ofs << "#include \"sylar/util/string_util.h\"" << std::endl;
    ofs << "#include \"json/json.h\"" << std::endl;
    ofs << std::endl;

    // 生成命名空间
    genNamespaces(ofs, true);

    // 生成日志对象
    ofs << "static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME(\"orm\");" << std::endl;
    ofs << std::endl;

    // 生成构造函数
    genConstructor(ofs, class_name);

    // 生成toString实现
    ofs << genToStringSrc(class_name) << std::endl;

    // 生成setter实现
    genSetterImplementations(ofs, class_name);

    // 生成DAO实现
    gen_dao_src(ofs);

    // 关闭命名空间
    genNamespaces(ofs, false);

    ofs << "// clang-format on" << std::endl;
  } catch (const std::exception& e) {
    SYLAR_LOG_ERROR(g_logger) << "Failed to generate source file: " << e.what();
  }
}

void Table::genConstructor(std::ofstream& ofs, const std::string& class_name) {
  ofs << GetAsClassName(class_name) << "::" << GetAsClassName(class_name) << "()" << std::endl;
  ofs << "  : ";

  auto cols = m_cols;
  std::sort(cols.begin(), cols.end(), [](const Column::ptr& a, const Column::ptr& b) {
    if (a->getDType() != b->getDType()) {
      return a->getDType() < b->getDType();
    }
    return a->getIndex() < b->getIndex();
  });

  bool is_first = true;
  for (const auto& col : cols) {
    if (!is_first) {
      ofs << std::endl << "  , ";
    }
    ofs << GetAsMemberName(col->getName()) << "(" << col->getDefaultValueString() << ")";
    is_first = false;
  }

  ofs << " {" << std::endl;
  ofs << "}" << std::endl;
  ofs << std::endl;
}

void Table::genSetterImplementations(std::ofstream& ofs, const std::string& class_name) {
  for (size_t i = 0; i < m_cols.size(); ++i) {
    ofs << m_cols[i]->getSetFunImpl(class_name, i) << std::endl;
  }
}

std::string Table::genToStringInc() {
  std::stringstream ss;
  ss << "std::string toJsonString() const;";
  return ss.str();
}

std::string Table::genToStringSrc(const std::string& class_name) {
  std::stringstream ss;
  ss << "std::string " << GetAsClassName(class_name) << "::toJsonString() const {" << std::endl;
  ss << "  Json::Value jvalue;" << std::endl;

  for (const auto& col : m_cols) {
    ss << "  jvalue[\"" << col->getName() << "\"] = ";
    if (col->getDType() == Column::TYPE_UINT64 || col->getDType() == Column::TYPE_INT64) {
      ss << "std::to_string(" << GetAsMemberName(col->getName()) << ");" << std::endl;
    } else if (col->getDType() == Column::TYPE_TIMESTAMP) {
      ss << "sylar::util::Time2Str(" << GetAsMemberName(col->getName()) << ");" << std::endl;
    } else {
      ss << GetAsMemberName(col->getName()) << ";" << std::endl;
    }
  }

  ss << "  return sylar::JsonUtil::ToString(jvalue);" << std::endl;
  ss << "}" << std::endl;
  return ss.str();
}

std::string Table::genToInsertSQL(const std::string& class_name) {
  std::stringstream ss;
  ss << "std::string " << GetAsClassName(class_name) << "::toInsertSQL() const {" << std::endl;
  ss << "  std::stringstream ss;" << std::endl;
  ss << "  ss << \"insert into " << m_name << "(";
  bool is_first = true;
  for (size_t i = 0; i < m_cols.size(); ++i) {
    if (m_cols[i]->isAutoIncrement()) {
      continue;
    }
    if (!is_first) {
      ss << ",";
    }
    ss << m_cols[i]->getName();
    is_first = false;
  }
  ss << ") values (\"" << std::endl;
  is_first = true;
  for (size_t i = 0; i < m_cols.size(); ++i) {
    if (m_cols[i]->isAutoIncrement()) {
      continue;
    }
    if (!is_first) {
      ss << "  ss << \",\";" << std::endl;
    }
    if (m_cols[i]->getDType() == Column::TYPE_STRING) {
      ss << "  ss << \"'\" << sylar::string_util::replace(" << GetAsMemberName(m_cols[i]->getName())
         << ", \"'\", \"''\") << \"'\";" << std::endl;
    } else {
      ss << "  ss << " << GetAsMemberName(m_cols[i]->getName()) << ";" << std::endl;
    }
    is_first = true;
  }
  ss << "  ss << \")\";" << std::endl;
  ss << "  return ss.str();" << std::endl;
  ss << "}" << std::endl;
  return ss.str();
}

std::string Table::genToUpdateSQL(const std::string& class_name) {
  std::stringstream ss;
  ss << "std::string " << GetAsClassName(class_name) << "::toUpdateSQL() const {" << std::endl;
  ss << "  std::stringstream ss;" << std::endl;
  ss << "  bool is_first = true;" << std::endl;
  ss << "  ss << \"update " << m_name << " set \";" << std::endl;
  for (size_t i = 0; i < m_cols.size(); ++i) {
    ss << "  if(_flags & " << (1ul << i) << "ul) {" << std::endl;
    ss << "    if(!is_first) {" << std::endl;
    ss << "      ss << \",\";" << std::endl;
    ss << "    }" << std::endl;
    ss << "    ss << \" " << m_cols[i]->getName() << " = ";
    if (m_cols[i]->getDType() == Column::TYPE_STRING) {
      ss << "'\" << sylar::string_util::replace(" << GetAsMemberName(m_cols[i]->getName())
         << ", \"'\", \"''\") << \"'\";" << std::endl;
    } else {
      ss << "\" << " << GetAsMemberName(m_cols[i]->getName()) << ";" << std::endl;
    }
    ss << "    is_first = false;" << std::endl;
    ss << "  }" << std::endl;
  }
  ss << genWhere();
  ss << "  return ss.str();" << std::endl;
  ss << "}" << std::endl;
  return ss.str();
}

std::string Table::genToDeleteSQL(const std::string& class_name) {
  std::stringstream ss;
  ss << "std::string " << GetAsClassName(class_name) << "::toDeleteSQL() const {" << std::endl;
  ss << "  std::stringstream ss;" << std::endl;
  ss << "  ss << \"delete from " << m_name << "\";" << std::endl;
  ss << genWhere();
  ss << "  return ss.str();" << std::endl;
  ss << "}" << std::endl;
  return ss.str();
}

std::string Table::genWhere() const {
  std::stringstream ss;
  ss << "  ss << \" where ";
  auto pks = getPKs();

  for (size_t i = 0; i < pks.size(); ++i) {
    if (i) {
      ss << "  ss << \" and ";
    }
    ss << pks[i]->getName() << " = ";
    if (pks[i]->getDType() == Column::TYPE_STRING) {
      ss << "'\" << sylar::string_util::replace(" << GetAsMemberName(m_cols[i]->getName())
         << ", \"'\", \"''\") << \"'\";" << std::endl;
    } else {
      ss << "\" << " << GetAsMemberName(m_cols[i]->getName()) << ";" << std::endl;
    }
  }
  return ss.str();
}

std::vector<Column::ptr> Table::getPKs() const {
  std::vector<Column::ptr> cols;
  for (auto& i : m_idxs) {
    if (i->isPK()) {
      for (auto& n : i->getCols()) {
        cols.push_back(getCol(n));
      }
    }
  }
  return cols;
}

Column::ptr Table::getCol(const std::string& name) const {
  for (auto& i : m_cols) {
    if (i->getName() == name) {
      return i;
    }
  }
  return nullptr;
}

void Table::gen_dao_inc(std::ofstream& ofs) {
  std::string class_name = m_name + m_subfix;
  std::string class_name_dao = class_name + "_dao";

  ofs << "class " << GetAsClassName(class_name_dao) << " {" << std::endl;
  ofs << "public:" << std::endl;
  ofs << " using ptr = std::shared_ptr<" << GetAsClassName(class_name_dao) << ">;" << std::endl;

  // 生成基本CRUD操作
  genBasicCRUD(ofs, class_name);

  // 生成索引相关操作
  genIndexOperations(ofs, class_name);

  // 生成建表操作
  genCreateTableOperations(ofs);

  ofs << "};" << std::endl;
}

void Table::genBasicCRUD(std::ofstream& ofs, const std::string& class_name) {
  ofs << "  static int Update(" << GetAsClassName(class_name) << "::ptr info, " << m_updateclass
      << "::ptr conn);" << std::endl;
  ofs << "  static int Insert(" << GetAsClassName(class_name) << "::ptr info, " << m_updateclass
      << "::ptr conn);" << std::endl;
  ofs << "  static int InsertOrUpdate(" << GetAsClassName(class_name) << "::ptr info, "
      << m_updateclass << "::ptr conn);" << std::endl;
  ofs << "  static int Delete(" << GetAsClassName(class_name) << "::ptr info, " << m_updateclass
      << "::ptr conn);" << std::endl;

  auto pks = getPKs();
  ofs << "  static int Delete(";
  for (const auto& pk : pks) {
    ofs << "const " << pk->getDTypeString() << "& " << GetAsVariable(pk->getName()) << ", ";
  }
  ofs << m_updateclass << "::ptr conn);" << std::endl;
}

void Table::genIndexOperations(std::ofstream& ofs, const std::string& class_name) {
  // 生成基于索引的删除操作
  for (const auto& idx : m_idxs) {
    if (idx->getDType() == Index::TYPE_UNIQ || idx->getDType() == Index::TYPE_PK ||
        idx->getDType() == Index::TYPE_INDEX) {
      genIndexDeleteOperation(ofs, class_name, idx);
    }
  }

  // 生成查询操作
  ofs << "  static int QueryAll(std::vector<" << GetAsClassName(class_name) << "::ptr>& results, "
      << m_queryclass << "::ptr conn);" << std::endl;

  auto pks = getPKs();
  ofs << "  static " << GetAsClassName(class_name) << "::ptr Query(";
  for (const auto& pk : pks) {
    ofs << "const " << pk->getDTypeString() << "& " << GetAsVariable(pk->getName()) << ", ";
  }
  ofs << m_queryclass << "::ptr conn);" << std::endl;

  // 生成基于索引的查询操作
  for (const auto& idx : m_idxs) {
    if (idx->getDType() == Index::TYPE_UNIQ) {
      genUniqueIndexQueryOperation(ofs, class_name, idx);
    } else if (idx->getDType() == Index::TYPE_INDEX) {
      genIndexQueryOperation(ofs, class_name, idx);
    }
  }
}

void Table::genIndexDeleteOperation(std::ofstream& ofs, const std::string& class_name,
                                    const Index::ptr& idx) {
  ofs << "  static int Delete";
  std::string tmp = "by";
  for (const auto& col : idx->getCols()) {
    tmp += "_" + col;
  }
  ofs << GetAsClassName(tmp) << "(";
  for (const auto& col : idx->getCols()) {
    auto d = getCol(col);
    ofs << "const " << d->getDTypeString() << "& " << GetAsVariable(d->getName()) << ", ";
  }
  ofs << m_updateclass << "::ptr conn);" << std::endl;
}

void Table::genUniqueIndexQueryOperation(std::ofstream& ofs, const std::string& class_name,
                                         const Index::ptr& idx) {
  ofs << "  static " << GetAsClassName(class_name) << "::ptr Query";
  std::string tmp = "by";
  for (const auto& col : idx->getCols()) {
    tmp += "_" + col;
  }
  ofs << GetAsClassName(tmp) << "(";
  for (const auto& col : idx->getCols()) {
    auto d = getCol(col);
    ofs << "const " << d->getDTypeString() << "& " << GetAsVariable(d->getName()) << ", ";
  }
  ofs << m_queryclass << "::ptr conn);" << std::endl;
}

void Table::genIndexQueryOperation(std::ofstream& ofs, const std::string& class_name,
                                   const Index::ptr& idx) {
  ofs << "  static int Query";
  std::string tmp = "by";
  for (const auto& col : idx->getCols()) {
    tmp += "_" + col;
  }
  ofs << GetAsClassName(tmp) << "(";
  ofs << "std::vector<" << GetAsClassName(class_name) << "::ptr>& results, ";
  for (const auto& col : idx->getCols()) {
    auto d = getCol(col);
    ofs << "const " << d->getDTypeString() << "& " << GetAsVariable(d->getName()) << ", ";
  }
  ofs << m_queryclass << "::ptr conn);" << std::endl;
}

void Table::genCreateTableOperations(std::ofstream& ofs) {
  ofs << "  static int CreateTableSQLite3(" << m_dbclass << "::ptr info);" << std::endl;
  ofs << "  static int CreateTableMySQL(" << m_dbclass << "::ptr info);" << std::endl;
}

void Table::gen_dao_src(std::ofstream& ofs) {
  std::string class_name = m_name + m_subfix;
  std::string class_name_dao = class_name + "_dao";

  // 生成Update实现
  genUpdateImpl(ofs, class_name, class_name_dao);

  // 生成Insert实现
  genInsertImpl(ofs, class_name, class_name_dao);

  // 生成InsertOrUpdate实现
  genInsertOrUpdateImpl(ofs, class_name, class_name_dao);

  // 生成Delete实现
  genDeleteImpl(ofs, class_name, class_name_dao);

  // 生成基于索引的Delete实现
  genIndexDeleteImpl(ofs, class_name, class_name_dao);

  // 生成QueryAll实现
  genQueryAllImpl(ofs, class_name, class_name_dao);

  // 生成Query实现
  genQueryImpl(ofs, class_name, class_name_dao);

  // 生成基于索引的Query实现
  genIndexQueryImpl(ofs, class_name, class_name_dao);

  // 生成建表实现
  genCreateTableImpl(ofs, class_name_dao);
}

void Table::genUpdateImpl(std::ofstream& ofs, const std::string& class_name,
                          const std::string& class_name_dao) {
  ofs << "int " << GetAsClassName(class_name_dao) << "::Update(" << GetAsClassName(class_name)
      << "::ptr info, " << m_updateclass << "::ptr conn) {" << std::endl;

  // 生成SQL语句
  std::stringstream sql;
  sql << "update " << m_name << " set";
  auto pks = getPKs();
  bool is_first = true;

  for (const auto& col : m_cols) {
    if (std::find(pks.begin(), pks.end(), col) != pks.end()) {
      continue;
    }
    if (!is_first) {
      sql << ",";
    }
    sql << " " << col->getName() << " = ?";
    is_first = false;
  }

  sql << " where";
  is_first = true;
  for (const auto& pk : pks) {
    if (!is_first) {
      sql << " and";
    }
    sql << " " << pk->getName() << " = ?";
    is_first = false;
  }

  ofs << "  std::string sql = \"" << sql.str() << "\";" << std::endl;

  // 生成预处理语句
  genPrepareStatement(ofs, "conn->getErrno()");

  // 绑定参数
  int idx = 1;
  for (const auto& col : m_cols) {
    if (std::find(pks.begin(), pks.end(), col) != pks.end()) {
      continue;
    }
    ofs << "  stmt->" << col->getBindString() << "(" << idx << ", ";
    ofs << "info->" << GetAsMemberName(col->getName()) << ");" << std::endl;
    ++idx;
  }

  for (const auto& pk : pks) {
    ofs << "  stmt->" << pk->getBindString() << "(" << idx << ", ";
    ofs << "info->" << GetAsMemberName(pk->getName()) << ");" << std::endl;
    ++idx;
  }

  ofs << "  return stmt->execute();" << std::endl;
  ofs << "}" << std::endl << std::endl;
}

void Table::genInsertImpl(std::ofstream& ofs, const std::string& class_name,
                          const std::string& class_name_dao) {
  ofs << "int " << GetAsClassName(class_name_dao) << "::Insert(" << GetAsClassName(class_name)
      << "::ptr info, " << m_updateclass << "::ptr conn) {" << std::endl;

  // 生成SQL语句
  std::stringstream sql;
  sql << "insert into " << m_name << " (";
  bool is_first = true;
  Column::ptr auto_inc;

  for (const auto& col : m_cols) {
    if (col->isAutoIncrement()) {
      auto_inc = col;
      continue;
    }
    if (!is_first) {
      sql << ", ";
    }
    sql << col->getName();
    is_first = false;
  }

  sql << ") values (";
  is_first = true;
  for (const auto& col : m_cols) {
    if (col->isAutoIncrement()) {
      continue;
    }
    if (!is_first) {
      sql << ", ";
    }
    sql << "?";
    is_first = false;
  }
  sql << ")";

  ofs << "  std::string sql = \"" << sql.str() << "\";" << std::endl;

  // 生成预处理语句
  genPrepareStatement(ofs, "conn->getErrno()");

  // 绑定参数
  int idx = 1;
  for (const auto& col : m_cols) {
    if (col->isAutoIncrement()) {
      continue;
    }
    ofs << "  stmt->" << col->getBindString() << "(" << idx << ", ";
    ofs << "info->" << GetAsMemberName(col->getName()) << ");" << std::endl;
    ++idx;
  }

  ofs << "  int rt = stmt->execute();" << std::endl;
  if (auto_inc) {
    ofs << "  if(rt == 0) {" << std::endl;
    ofs << "    info->" << GetAsMemberName(auto_inc->getName()) << " = conn->getLastInsertId();"
        << std::endl;
    ofs << "  }" << std::endl;
  }
  ofs << "  return rt;" << std::endl;
  ofs << "}" << std::endl << std::endl;
}

void Table::genPrepareStatement(std::ofstream& ofs, const std::string& error_return) {
  ofs << "  auto stmt = conn->prepare(sql);" << std::endl;
  ofs << "  if(!stmt) {" << std::endl;
  ofs << "    SYLAR_LOG_ERROR(g_logger) << \"stmt=\" << sql" << std::endl
      << "         << \" errno=\" << conn->getErrno()" << " << \" errstr=\" << conn->getErrStr();"
      << std::endl
      << "    return " << error_return << ";" << std::endl;
  ofs << "  }" << std::endl;
}

void Table::genInsertOrUpdateImpl(std::ofstream& ofs, const std::string& class_name,
                                  const std::string& class_name_dao) {
  ofs << "int " << GetAsClassName(class_name_dao) << "::InsertOrUpdate("
      << GetAsClassName(class_name) << "::ptr info, " << m_updateclass << "::ptr conn) {"
      << std::endl;
  Column::ptr auto_inc;
  for (auto& i : m_cols) {
    if (i->isAutoIncrement()) {
      auto_inc = i;
    }
  }
  if (auto_inc) {
    ofs << "  if(info->" << GetAsMemberName(auto_inc->getName()) << " == 0) {" << std::endl;
    ofs << "    return Insert(info, conn);" << std::endl;
    ofs << "  }" << std::endl;
  }
  ofs << "  std::string sql = \"replace into " << m_name << " (";
  bool is_first = true;
  for (auto& i : m_cols) {
    if (!is_first) {
      ofs << ", ";
    }
    ofs << i->getName();
    is_first = false;
  }

  ofs << ") values (";
  is_first = true;
  for (auto& i : m_cols) {
    (void)i;
    if (!is_first) {
      ofs << ", ";
    }
    ofs << "?";
    is_first = false;
  }
  ofs << ")\";" << std::endl;

  genPrepareStatement(ofs, "conn->getErrno()");
  int idx = 1;
  for (auto& i : m_cols) {
    ofs << "  stmt->" << i->getBindString() << "(" << idx << ", ";
    ofs << "info->" << GetAsMemberName(i->getName()) << ");" << std::endl;
    ++idx;
  }
  ofs << "  return stmt->execute();" << std::endl;
  ofs << "}" << std::endl << std::endl;
}

void Table::genDeleteImpl(std::ofstream& ofs, const std::string& class_name,
                          const std::string& class_name_dao) {
  ofs << "int " << GetAsClassName(class_name_dao) << "::Delete(" << GetAsClassName(class_name)
      << "::ptr info, " << m_updateclass << "::ptr conn) {" << std::endl;

  ofs << "  std::string sql = \"delete from " << m_name << " where";
  bool is_first = true;
  auto pks = getPKs();
  for (auto& i : pks) {
    if (!is_first) {
      ofs << " and";
    }
    ofs << " " << i->getName() << " = ?";
    is_first = false;
  }
  ofs << "\";" << std::endl;
  genPrepareStatement(ofs, "conn->getErrno()");
  int idx = 1;
  for (auto& i : pks) {
    ofs << "  stmt->" << i->getBindString() << "(" << idx << ", ";
    ofs << "info->" << GetAsMemberName(i->getName()) << ");" << std::endl;
    ++idx;
  }
  ofs << "  return stmt->execute();" << std::endl;
  ofs << "}" << std::endl << std::endl;
}

void Table::genIndexDeleteImpl(std::ofstream& ofs, const std::string& class_name,
                               const std::string& class_name_dao) {
  for (auto& i : m_idxs) {
    if (i->getDType() == Index::TYPE_UNIQ || i->getDType() == Index::TYPE_PK ||
        i->getDType() == Index::TYPE_INDEX) {
      ofs << "int " << GetAsClassName(class_name_dao) << "::Delete";
      std::string tmp = "by";
      for (auto& c : i->getCols()) {
        tmp += "_" + c;
      }
      ofs << GetAsClassName(tmp) << "(";
      for (auto& c : i->getCols()) {
        auto d = getCol(c);
        ofs << " const " << d->getDTypeString() << "& " << GetAsVariable(d->getName()) << ", ";
      }
      ofs << m_updateclass << "::ptr conn) {" << std::endl;
      ofs << "  std::string sql = \"delete from " << m_name << " where";
      bool is_first = true;
      for (auto& x : i->getCols()) {
        if (!is_first) {
          ofs << " and";
        }
        ofs << " " << x << " = ?";
        is_first = false;
      }
      ofs << "\";" << std::endl;
      genPrepareStatement(ofs, "conn->getErrno()");
      int idx = 1;
      for (auto& x : i->getCols()) {
        ofs << "  stmt->" << getCol(x)->getBindString() << "(" << idx << ", ";
        ofs << GetAsVariable(x) << ");" << std::endl;
      }
      ofs << "  return stmt->execute();" << std::endl;
      ofs << "}" << std::endl << std::endl;
    }
  }
}

void Table::genQueryAllImpl(std::ofstream& ofs, const std::string& class_name,
                            const std::string& class_name_dao) {
  ofs << "int " << GetAsClassName(class_name_dao) << "::QueryAll(std::vector<"
      << GetAsClassName(class_name) << "::ptr>& results, " << m_queryclass << "::ptr conn) {"
      << std::endl;
  ofs << "  std::string sql = \"select ";
  bool is_first = true;
  for (auto& i : m_cols) {
    if (!is_first) {
      ofs << ", ";
    }
    ofs << i->getName();
    is_first = false;
  }
  ofs << " from " << m_name << "\";" << std::endl;
  genPrepareStatement(ofs, "conn->getErrno()");
  ofs << "  auto rt = stmt->query();" << std::endl;
  ofs << "  if(!rt) {" << std::endl;
  ofs << "    return stmt->getErrno();" << std::endl;
  ofs << "  }" << std::endl;
  ofs << "  while (rt->next()) {" << std::endl;
  ofs << "    " << GetAsClassName(class_name) << "::ptr v(new " << GetAsClassName(class_name)
      << ");" << std::endl;

#define PARSE_OBJECT(prefix)                                                       \
  for (size_t i = 0; i < m_cols.size(); ++i) {                                     \
    ofs << prefix "v->" << GetAsMemberName(m_cols[i]->getName()) << " = ";         \
    ofs << "rt->" << m_cols[i]->getGetString() << "(" << (i) << ");" << std::endl; \
  }
  PARSE_OBJECT("    ");
  ofs << "    results.push_back(v);" << std::endl;
  ofs << "  }" << std::endl;
  ofs << "  return 0;" << std::endl;
  ofs << "}" << std::endl << std::endl;
}

void Table::genQueryImpl(std::ofstream& ofs, const std::string& class_name,
                         const std::string& class_name_dao) {
  ofs << GetAsClassName(class_name) << "::ptr " << GetAsClassName(class_name_dao) << "::Query(";
  for (auto& i : getPKs()) {
    ofs << " const " << i->getDTypeString() << "& " << GetAsVariable(i->getName()) << ", ";
  }
  ofs << m_queryclass << "::ptr conn) {" << std::endl;

  ofs << "  std::string sql = \"select ";
  bool is_first = true;
  for (auto& i : m_cols) {
    if (!is_first) {
      ofs << ", ";
    }
    ofs << i->getName();
    is_first = false;
  }
  ofs << " from " << m_name << " where";
  is_first = true;
  for (auto& i : getPKs()) {
    if (!is_first) {
      ofs << " and";
    }
    ofs << " " << i->getName() << " = ?";
    is_first = false;
  }
  ofs << "\";" << std::endl;

  genPrepareStatement(ofs, "nullptr");
  int idx = 1;
  for (auto& i : getPKs()) {
    ofs << "  stmt->" << i->getBindString() << "(" << idx << ", ";
    ofs << GetAsVariable(i->getName()) << ");" << std::endl;
    ++idx;
  }
  ofs << "  auto rt = stmt->query();" << std::endl;
  ofs << "  if(!rt) {" << std::endl;
  ofs << "    return nullptr;" << std::endl;
  ofs << "  }" << std::endl;
  ofs << "  if(!rt->next()) {" << std::endl;
  ofs << "    return nullptr;" << std::endl;
  ofs << "  }" << std::endl;
  ofs << "  " << GetAsClassName(class_name) << "::ptr v(new " << GetAsClassName(class_name) << ");"
      << std::endl;
  PARSE_OBJECT("  ");
  ofs << "  return v;" << std::endl;
  ofs << "}" << std::endl << std::endl;
}

void Table::genIndexQueryImpl(std::ofstream& ofs, const std::string& class_name,
                              const std::string& class_name_dao) {
  for (auto& i : m_idxs) {
    if (i->getDType() == Index::TYPE_UNIQ) {
      ofs << "" << GetAsClassName(class_name) << "::ptr " << GetAsClassName(class_name_dao)
          << "::Query";
      std::string tmp = "by";
      for (auto& c : i->getCols()) {
        tmp += "_" + c;
      }
      ofs << GetAsClassName(tmp) << "(";
      for (auto& c : i->getCols()) {
        auto d = getCol(c);
        ofs << " const " << d->getDTypeString() << "& " << GetAsVariable(d->getName()) << ", ";
      }
      ofs << m_queryclass << "::ptr conn) {" << std::endl;

      ofs << "  std::string sql = \"select ";
      bool is_first = true;
      for (auto& i : m_cols) {
        if (!is_first) {
          ofs << ", ";
        }
        ofs << i->getName();
        is_first = false;
      }
      ofs << " from " << m_name << " where";
      is_first = true;
      for (auto& x : i->getCols()) {
        if (!is_first) {
          ofs << " and";
        }
        ofs << " " << x << " = ?";
        is_first = false;
      }
      ofs << "\";" << std::endl;
      genPrepareStatement(ofs, "nullptr");

      int idx = 1;
      for (auto& x : i->getCols()) {
        ofs << "  stmt->" << getCol(x)->getBindString() << "(" << idx << ", ";
        ofs << GetAsVariable(x) << ");" << std::endl;
        ++idx;
      }
      ofs << "  auto rt = stmt->query();" << std::endl;
      ofs << "  if(!rt) {" << std::endl;
      ofs << "    return nullptr;" << std::endl;
      ofs << "  }" << std::endl;
      ofs << "  if(!rt->next()) {" << std::endl;
      ofs << "    return nullptr;" << std::endl;
      ofs << "  }" << std::endl;
      ofs << "  " << GetAsClassName(class_name) << "::ptr v(new " << GetAsClassName(class_name)
          << ");" << std::endl;
      PARSE_OBJECT("  ");
      ofs << "  return v;" << std::endl;
      ofs << "}" << std::endl << std::endl;
    } else if (i->getDType() == Index::TYPE_INDEX) {
      ofs << "int " << GetAsClassName(class_name_dao) << "::Query";
      std::string tmp = "by";
      for (auto& c : i->getCols()) {
        tmp += "_" + c;
      }
      ofs << GetAsClassName(tmp) << "(";
      ofs << "std::vector<" << GetAsClassName(class_name) << "::ptr>& results, ";
      for (auto& c : i->getCols()) {
        auto d = getCol(c);
        ofs << " const " << d->getDTypeString() << "& " << GetAsVariable(d->getName()) << ", ";
      }
      ofs << m_queryclass << "::ptr conn) {" << std::endl;

      ofs << "  std::string sql = \"select ";
      bool is_first = true;
      for (auto& i : m_cols) {
        if (!is_first) {
          ofs << ", ";
        }
        ofs << i->getName();
        is_first = false;
      }
      ofs << " from " << m_name << " where";
      is_first = true;
      for (auto& x : i->getCols()) {
        if (!is_first) {
          ofs << " and";
        }
        ofs << " " << x << " = ?";
        is_first = false;
      }
      ofs << "\";" << std::endl;
      genPrepareStatement(ofs, "conn->getErrno()");

      int idx = 1;
      for (auto& x : i->getCols()) {
        ofs << "  stmt->" << getCol(x)->getBindString() << "(" << idx << ", ";
        ofs << GetAsVariable(x) << ");" << std::endl;
        ++idx;
      }
      ofs << "  auto rt = stmt->query();" << std::endl;
      ofs << "  if(!rt) {" << std::endl;
      ofs << "    return 0;" << std::endl;
      ofs << "  }" << std::endl;
      ofs << "  while (rt->next()) {" << std::endl;
      ofs << "    " << GetAsClassName(class_name) << "::ptr v(new " << GetAsClassName(class_name)
          << ");" << std::endl;
      PARSE_OBJECT("    ");
      ofs << "    results.push_back(v);" << std::endl;
      ofs << "  };" << std::endl;
      ofs << "  return 0;" << std::endl;
      ofs << "}" << std::endl << std::endl;
    }
  }
}

void Table::genCreateTableImpl(std::ofstream& ofs, const std::string& class_name_dao) {
  ofs << "int " << GetAsClassName(class_name_dao) << "::CreateTableSQLite3(" << m_dbclass
      << "::ptr conn) {" << std::endl;
  ofs << "  return conn->execute(\"CREATE TABLE " << m_name << "(\"" << std::endl;
  bool is_first = true;
  bool has_auto_increment = false;
  for (auto& i : m_cols) {
    if (!is_first) {
      ofs << ",\"" << std::endl;
    }
    ofs << "      \"" << i->getName() << " " << i->getSQLite3TypeString();
    if (i->isAutoIncrement()) {
      ofs << " PRIMARY KEY AUTOINCREMENT";
      has_auto_increment = true;
    } else {
      ofs << " NOT NULL DEFAULT " << i->getSQLite3Default();
    }
    is_first = false;
  }
  if (!has_auto_increment) {
    ofs << ", PRIMARY KEY(";
    is_first = true;
    for (auto& i : getPKs()) {
      if (!is_first) {
        ofs << ", ";
      }
      ofs << i->getName();
    }
    ofs << ")";
  }
  ofs << ");\"" << std::endl;
  for (auto& i : m_idxs) {
    if (i->getDType() == Index::TYPE_PK) {
      continue;
    }
    ofs << "      \"CREATE";
    if (i->getDType() == Index::TYPE_UNIQ) {
      ofs << " UNIQUE";
    }
    ofs << " INDEX " << m_name;
    for (auto& x : i->getCols()) {
      ofs << "_" << x;
    }
    ofs << " ON " << m_name << "(";
    is_first = true;
    for (auto& x : i->getCols()) {
      if (!is_first) {
        ofs << ",";
      }
      ofs << x;
      is_first = false;
    }
    ofs << ");\"" << std::endl;
  }
  ofs << "      );" << std::endl;
  ofs << "}" << std::endl << std::endl;

  ofs << "int " << GetAsClassName(class_name_dao) << "::CreateTableMySQL(" << m_dbclass
      << "::ptr conn) {" << std::endl;
  ofs << "  return conn->execute(\"CREATE TABLE " << m_name << "(\"" << std::endl;
  is_first = true;
  for (auto& i : m_cols) {
    if (!is_first) {
      ofs << ",\"" << std::endl;
    }
    ofs << "      \"`" << i->getName() << "` " << i->getMySQLTypeString();
    if (i->isAutoIncrement()) {
      ofs << " AUTO_INCREMENT";
      has_auto_increment = true;
    } else {
      ofs << " NOT NULL DEFAULT " << i->getSQLite3Default();
    }

    if (!i->getUpdate().empty()) {
      ofs << " ON UPDATE " << i->getUpdate() << " ";
    }
    if (!i->getDesc().empty()) {
      ofs << " COMMENT '" << i->getDesc() << "'";
    }
    is_first = false;
  }
  ofs << ",\"" << std::endl << "      \"PRIMARY KEY(";
  is_first = true;
  for (auto& i : getPKs()) {
    if (!is_first) {
      ofs << ", ";
    }
    ofs << "`" << i->getName() << "`";
  }
  ofs << ")";
  for (auto& i : m_idxs) {
    if (i->getDType() == Index::TYPE_PK) {
      continue;
    }
    ofs << ",\"" << std::endl;
    if (i->getDType() == Index::TYPE_UNIQ) {
      ofs << "      \"UNIQUE ";
    } else {
      ofs << "      \"";
    }
    ofs << "KEY `" << m_name;
    for (auto& x : i->getCols()) {
      ofs << "_" << x;
    }
    ofs << "` (";
    is_first = true;
    for (auto& x : i->getCols()) {
      if (!is_first) {
        ofs << ",";
      }
      ofs << "`" << x << "`";
      is_first = false;
    }
    ofs << ")";
  }
  ofs << ")";
  if (!m_desc.empty()) {
    ofs << " COMMENT='" << m_desc << "'";
  }
  ofs << "\");" << std::endl;
  ofs << "}";
}

}   // namespace orm
}   // namespace sylar