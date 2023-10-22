#ifndef CHAT_INCLUDE_DAO_USERMODEL_H
#define CHAT_INCLUDE_DAO_USERMODEL_H

#include "include/server/dao/User.h"
#include "include/server/db/db.h"

#include <vector>

class UserModel {
 public:
  bool updateState(User& user);
  bool insertUser(User& user);
  bool resetState();
  bool delUser(User& user);
  User queryUserById(int id);
  std::vector<User> queryAllUser();
};

#endif  // CHAT_INCLUDE_DAO_USERMODEL_H