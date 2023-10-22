
#include "include/server/dao/FriendModel.h"
#include "include/server/db/db.h"

#include <stdio.h>
#include <string.h>

bool FriendModel::insertFriend(Friend f) {
  char sql[1024] = {0};
  sprintf(sql, "INSERT INTO friend(userid, friendid) VALUES('%d', '%d')",
          f.getUserId(), f.getFriendId());
  MySQL mysql;
  if (mysql.connect()) {
    if (mysql.update(sql, strlen(sql))) {
      return true;
    }
  }

  return false;
}

bool FriendModel::delFriend(Friend f) {
  char sql[1024] = {0};
  sprintf(sql, "DELETE FORM friend WHERE userid='%d' and friendid='%d'",
          f.getUserId(), f.getFriendId());
  MySQL mysql;
  if (mysql.connect()) {
    if (mysql.update(sql, strlen(sql))) {
      return true;
    }
  }

  return false;
}

// 查找id对应的所有好友
std::vector<User> FriendModel::queryFriendById(int id) {
  char sql[1024] = {0};
  sprintf(sql,
          "SELECT b.friendid, a.name, a.state FROM user a INNER JOIN "
          "friend b ON a.id = b.friendid WHERE userid='%d'",
          id);

  std::vector<User> friends;
  MySQL mysql;
  if (mysql.connect()) {
    MYSQL_RES* queryRes = mysql.query(sql, strlen(sql));
    if (queryRes != nullptr) {
      MYSQL_ROW row;
      while ((row = mysql_fetch_row(queryRes)) != nullptr) {
        User u;
        u.setId(atoi(row[0]));
        u.setName(row[1]);
        u.setState(row[2]);
        friends.push_back(u);
      }
    }
  }
  return friends;
}