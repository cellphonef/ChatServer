#include "include/server/dao/GroupModel.h"

#include "include/server/db/db.h"

#include <string.h>

// 创建一个新群
bool GroupModel::insertGroup(Group& group) {
  char sql[1024] = {0};
  sprintf(sql, "INSERT INTO allgroup(groupname, groupdesc) VALUES('%s', '%s')",
          group.getGroupname().c_str(), group.getGroupdesc().c_str());

  MySQL mysql;
  if (mysql.connect()) {
    if (mysql.update(sql, strlen(sql))) {
      group.setGroupid(mysql_insert_id(mysql.getConn()));
      return true;
    }
  }
  return false;
}

// 删除一个群
bool GroupModel::delGroup() {}

// 根据userid查找该用户所加的群
std::vector<Group> GroupModel::queryGroupById(int userid) {
  char sql[1024] = {0};
  sprintf(sql,
          "SELECT b.groupid, b.groupname, b.groupdesc FROM groupuser a INNER "
          "JOIN allgroup b ON a.groupid = "
          "b.groupid WHERE a.userid = '%d'",
          userid);

  MySQL mysql;
  std::vector<Group> groups;
  if (mysql.connect()) {
    MYSQL_RES* queryRes = mysql.query(sql, strlen(sql));
    if (queryRes != nullptr) {
      MYSQL_ROW row;
      while ((row = mysql_fetch_row(queryRes)) != nullptr) {
        Group g;
        g.setGroupid(atoi(row[0]));
        g.setGroupname(row[1]);
        g.setGroupdesc(row[2]);
        groups.push_back(g);
      }
    }
  }
  return groups;
}

Group GroupModel::queryGroupInfoById(int groupid) {
  char sql[1024] = {0};
  sprintf(sql,
          "SELECT sq.groupid, sq.groupname, sq.groupdesc, sq.userid, "
          "sq.grouprole, u.name, u.state FROM (SELECT a.groupid, a.groupname, "
          "a.groupdesc, b.userid, b.grouprole FROM allgroup a INNER JOIN "
          "groupuser b ON a.groupid=b.groupid WHERE a.groupid='%d') AS sq "
          "INNER JOIN user u ON sq.userid=u.id",
          groupid);

  MySQL mysql;
  Group group;
  if (mysql.connect()) {
    MYSQL_RES* queryRes = mysql.query(sql, strlen(sql));
    if (queryRes != nullptr) {
      MYSQL_ROW row;
      while ((row = mysql_fetch_row(queryRes)) != nullptr) {
        group.setGroupid(atoi(row[0]));
        group.setGroupname(row[1]);
        group.setGroupdesc(row[2]);
        GroupUser gu;
        gu.setId(atoi(row[3]));
        gu.setRole(row[4]);
        gu.setName(row[5]);
        gu.setGroupid(atoi(row[0]));
        gu.setState(row[6]);
        group.getGroupusers().push_back(gu);
      }
    }
  }
  return group;
}