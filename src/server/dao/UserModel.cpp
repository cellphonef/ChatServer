#include "include/server/dao/UserModel.h"

#include <string.h>
#include <iostream>

bool UserModel::insertUser(User& user) {
  char sql[1024] = {0};

  sprintf(sql, "INSERT INTO User(name, passwd, state) values('%s', '%s', '%s')",
          user.getName().c_str(), user.getPasswd().c_str(),
          user.getState().c_str());

  MySQL mysql;
  if (mysql.connect()) {
    if (mysql.update(sql, strlen(sql))) {
      user.setId(mysql_insert_id(mysql.getConn()));
      return true;
    }
  }
  return false;
}
bool UserModel::delUser(User& user) {
  char sql[1024] = {0};

  sprintf(sql, "DELETE FROM User WHERE user");

  MySQL mysql;
  if (mysql.connect()) {
    if (mysql.update(sql, strlen(sql))) {
      return true;
    }
  }
  return false;
}

bool UserModel::updateState(User& user) {
  char sql[1024] = {0};

  sprintf(sql, "UPDATE User SET state='%s' where id='%d'",
          user.getState().c_str(), user.getId());

  MySQL mysql;
  if (mysql.connect()) {
    if (mysql.update(sql, strlen(sql))) {
      return true;
    }
  }
  return false;
}

std::vector<User> UserModel::queryAllUser() {
  char sql[1024] = {0};

  sprintf(sql, "SELECT * FROM User");

  std::vector<User> users;
  MySQL mysql;
  if (mysql.connect()) {
    MYSQL_RES* resultSet = mysql.query(sql, strlen(sql));

    if (resultSet != nullptr) {
      while (MYSQL_ROW row = mysql_fetch_row(resultSet)) {
        User u;
        u.setId(atoi(row[0]));
        u.setName(row[1]);
        u.setPasswd(row[2]);
        u.setState(row[3]);
        users.emplace_back(u);
      }
    }
  }
  return users;
}

bool UserModel::resetState() {
  char sql[1024] = {0};
  sprintf(sql, "UPDATE User SET state='offline' where state='online'");

  MySQL mysql;
  if (mysql.connect()) {
    if (mysql.update(sql, strlen(sql))) {
      return true;
    }
  }

  return false;
}

User UserModel::queryUserById(int id) {
  char sql[1024] = {0};
  sprintf(sql, "SELECT * FROM User WHERE id='%d'", id);

  MySQL mysql;
  if (mysql.connect()) {
    MYSQL_RES* resultSet = mysql.query(sql, strlen(sql));

    if (resultSet != nullptr) {
      MYSQL_ROW row = mysql_fetch_row(resultSet);
      User u;
      u.setId(atoi(row[0]));
      u.setName(row[1]);
      u.setPasswd(row[2]);
      u.setState(row[3]);
      mysql_free_result(resultSet);
      return u;
    }
  }
  return User();
}