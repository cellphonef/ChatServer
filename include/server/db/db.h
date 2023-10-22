#ifndef CHAT_INCLUDE_DB_DB_H
#define CHAT_INCLUDE_DB_DB_H

#include <mysql/mysql.h>

class MySQL {
 public:
  MySQL();
  ~MySQL();

  bool connect();

  // 增删改
  bool update(const char* sql, int length);

  // 查
  MYSQL_RES* query(const char* sql, int length);

  MYSQL* getConn() const;

 private:
  MYSQL* conn_;
};

#endif  // CHAT_INCLUDE_DB_DB_H
