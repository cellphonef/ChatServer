#ifndef CHAT_INCLUDE_SERVER_DAO_FRIENDMODEL_H
#define CHAT_INCLUDE_SERVER_DAO_FRIENDMODEL_H

#include "include/server/dao/Friend.h"
#include "include/server/dao/FriendModel.h"
#include "include/server/dao/User.h"

#include <vector>

class FriendModel {
 public:
  bool insertFriend(Friend f);
  bool delFriend(Friend f);
  std::vector<User> queryFriendById(int id);
};

#endif  // CHAT_INCLUDE_SERVER_DAO_OFFLINEMESSAGEMODEL_H