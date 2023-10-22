#ifndef CHAT_INCLUDE_SERVER_DAO_GROUPUSER_H
#define CHAT_INCLUDE_SERVER_DAO_GROUPUSER_H

#include "include/server/dao/User.h"

#include <string>

class GroupUser : public User {
 public:
  void setGroupid(int groupid) { groupid_ = groupid; }
  void setRole(std::string role) { role_ = role; }

  int getGroupid() const { return groupid_; }
  std::string getRole() const { return role_; }

 private:
  int groupid_;
  std::string role_;
};

#endif  // CHAT_INCLUDE_SERVER_DAO_GROUPUSERMODEL_H