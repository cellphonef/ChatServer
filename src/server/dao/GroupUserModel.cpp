#include "include/server/dao/GroupUserModel.h"
#include "include/server/db/db.h"

#include <string.h>

bool GroupUserModel::insertGroupUser(GroupUser gu) {
  char sql[1024] = {0};
  sprintf(sql,
          "INSERT INTO groupuser(groupid, userid, grouprole) VALUES('%d', "
          "'%d', '%s')",
          gu.getGroupid(), gu.getId(), gu.getRole().c_str());

  MySQL mysql;
  if (mysql.connect()) {
    if (mysql.update(sql, strlen(sql))) {
      return true;
    }
  }
  return false;
}

bool GroupUserModel::delGroupUser() {}

std::vector<GroupUser> GroupUserModel::queryGroupUserById(int groupid) {
  char sql[1024] = {0};
  sprintf(sql,
          "SELECT b.id, b.name, b.state, a.grouprole FROM groupuser a INNER "
          "JOIN user b ON a.userid = b.id WHERE groupid = '%d'",
          groupid);

  MySQL mysql;
  std::vector<GroupUser> groupUsers;
  if (mysql.connect()) {
    MYSQL_RES* queryRes = mysql.query(sql, strlen(sql));
    if (queryRes != nullptr) {
      MYSQL_ROW row;
      while ((row = mysql_fetch_row(queryRes)) != nullptr) {
        GroupUser g;
        g.setId(atoi(row[0]));
        g.setName(row[1]);
        g.setState(row[2]);
        g.setRole(row[3]);
        
        groupUsers.push_back(g);
      }
    }
  }
  return groupUsers;
}

int GroupUserModel::queryCreatorById(int groupid) {
  char sql[1024] = {0};
  sprintf(sql,
          "SELECT userid FROM groupuser WHERE groupid = '%d' and grouprole='creator'",
          groupid);

  MySQL mysql;
  if (mysql.connect()) {
    MYSQL_RES* queryRes = mysql.query(sql, strlen(sql));
    if (queryRes != nullptr) {
      MYSQL_ROW row = mysql_fetch_row(queryRes);
      return atoi(row[0]);
    }
  }
  return -1;
}