#ifndef CHAT_INCLUDE_SERVER_DAO_GROUP_H
#define CHAT_INCLUDE_SERVER_DAO_GROUP_H

#include <include/server/dao/GroupUser.h>
#include <string>
#include <vector>

class Group {
 public:
  Group() : groupid_(-1) {}
  ~Group() = default;

  void setGroupid(int groupid) { groupid_ = groupid; }
  void setGroupname(std::string groupname) { groupname_ = groupname; }
  void setGroupdesc(std::string groupdesc) { groupdesc_ = groupdesc; }

  int getGroupid() const { return groupid_; }
  std::string getGroupname() const { return groupname_; }
  std::string getGroupdesc() const { return groupdesc_; }
  std::vector<GroupUser>& getGroupusers() { return groupusers_; }

 private:
  int groupid_;
  std::string groupname_;
  std::string groupdesc_;
  std::vector<GroupUser> groupusers_;
};

#endif  // CHAT_INCLUDE_SERVER_DAO_GROUP_H