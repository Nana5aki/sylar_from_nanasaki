/*
 * @Author: Nana5aki
 * @Date: 2025-05-23 21:22:13
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-05-24 20:07:37
 * @FilePath: /sylar_from_nanasaki/tests/test_orm.cpp
 */

#include "bin/orm_out/test/orm/user_info.h"
#include "sylar/db/sqlite3.h"
#include <iostream>

int main(int argc, char* argv[]) {
  sylar::DB::IDB::ptr db;
  db = sylar::DB::SQLite3::Create("abc.db");
  std::cout << "create table: " << test::orm::UserInfoDao::CreateTableSQLite3(db) << std::endl;
  
  for (int i = 0; i < 10; ++i) {
    test::orm::UserInfo::ptr u(new test::orm::UserInfo);
    u->setName("name_a" + std::to_string(i));
    u->setEmail("mail_a" + std::to_string(i));
    u->setPhone("phone_a" + std::to_string(i));
    u->setStatus(i % 2);

    std::cout << "i= " << i << " - " << test::orm::UserInfoDao::Insert(u, db);
    std::cout << " - " << u->toJsonString() << std::endl;
  }

  std::vector<test::orm::UserInfo::ptr> us;
  std::cout << "query_by_status: " << test::orm::UserInfoDao::QueryByStatus(us, 1, db) << std::endl;
  for (size_t i = 0; i < us.size(); ++i) {
    std::cout << "i=" << i << " - " << us[i]->toJsonString() << std::endl;
    us[i]->setName(us[i]->getName() + "_new");
    test::orm::UserInfoDao::Update(us[i], db);
  }

  std::cout << "delete: " << test::orm::UserInfoDao::DeleteByStatus(1, db) << std::endl;
  us.clear();
  std::cout << "query_by_status: " << test::orm::UserInfoDao::QueryByStatus(us, 0, db) << std::endl;
  for (size_t i = 0; i < us.size(); ++i) {
    std::cout << "i=" << i << " - " << us[i]->toJsonString() << std::endl;
  }

  us.clear();
  std::cout << "query_all: " << test::orm::UserInfoDao::QueryAll(us, db) << std::endl;
  for (size_t i = 0; i < us.size(); ++i) {
    std::cout << "i=" << i << " - " << us[i]->toJsonString() << std::endl;
  }
  return 0;
}