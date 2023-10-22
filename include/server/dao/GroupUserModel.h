#ifndef CHAT_INCLUDE_SERVER_DAO_GROUPUSERMODEL_H
#define CHAT_INCLUDE_SERVER_DAO_GROUPUSERMODEL_H

#include "include/server/dao/GroupUser.h"

#include <vector>

class GroupUserModel {
 public:
  bool insertGroupUser(GroupUser gu);
  bool delGroupUser();
  std::vector<GroupUser> queryGroupUserById(int groupid);
  int queryCreatorById(int groupid);
};

#endif  // CHAT_INCLUDE_SERVER_DAO_GROUPUSERMODEL_H