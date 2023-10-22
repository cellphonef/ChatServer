#include "include/server/dao/OfflineMessageModel.h"
#include "include/server/db/db.h"

#include <string.h>

bool OfflineMessageModel::insertMessage(OfflineMessage message) {
  char sql[1024] = {0};
  sprintf(sql, "INSERT INTO offlinemessage(userid, message) VALUES('%d', '%s')",
          message.getId(), message.getMessage().c_str());

  MySQL mysql;
  if (mysql.connect()) {
    if (mysql.update(sql, strlen(sql))) {
      return true;
    }
  }
  return false;
}

bool OfflineMessageModel::delMessage(OfflineMessage message) {
  char sql[1024] = {0};
  sprintf(sql, "DELETE FROM offlinemessage WHERE userid='%d'", message.getId());

  MySQL mysql;
  if (mysql.connect()) {
    if (mysql.update(sql, strlen(sql))) {
      return true;
    }
  }
  return false;
}

std::vector<std::string> OfflineMessageModel::queryMessage(int id) {
  char sql[1024] = {0};
  sprintf(sql, "SELECT message FROM offlinemessage WHERE userid = '%d'", id);

  std::vector<std::string> messages;
  MySQL mysql;
  if (mysql.connect()) {
    MYSQL_RES* res = mysql.query(sql, strlen(sql));
    if (res != nullptr) {  // 有离线信息
      MYSQL_ROW row;
      while ((row = mysql_fetch_row(res)) != nullptr) {
        messages.push_back(row[0]);
      }
    }
  }
  return messages;
}