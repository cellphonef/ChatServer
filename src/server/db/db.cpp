#include "include/server/db/db.h"
#include <mymuduo/Logging.h>

const char* host = "127.0.0.1";
const char* user = "root";
const char* passwd = "123456";
const char* db = "chat";

MySQL::MySQL() { conn_ = mysql_init(nullptr); }

MySQL::~MySQL() {
  if (conn_) mysql_close(conn_);
}

bool MySQL::connect() {
  if (!mysql_real_connect(conn_, host, user, passwd, db, 3306, nullptr, 0)) {
    LOG_INFO << "connect failed!";
    return false;
  }
  mysql_real_query(conn_, "set names gbk", 13);
  return true;
}

MYSQL_RES* MySQL::query(const char* sql, int length) {
  if (mysql_real_query(conn_, sql, length)) {
    LOG_INFO << sql << " mysql_real_query error : " << mysql_error(conn_);
    return nullptr;
  }
  return mysql_use_result(conn_);
}

bool MySQL::update(const char* sql, int length) {
  if (mysql_real_query(conn_, sql, length)) {
    LOG_INFO << sql << " mysql_real_query error : " << mysql_error(conn_);
    return false;
  }
  return true;
}

MYSQL* MySQL::getConn() const { return conn_; }