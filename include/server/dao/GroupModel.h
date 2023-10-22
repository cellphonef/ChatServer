#ifndef CHAT_INCLUDE_SERVER_DAO_GROUPMODEL_H
#define CHAT_INCLUDE_SERVER_DAO_GROUPMODEL_H

#include "include/server/dao/Group.h"

#include <vector>

class GroupModel {
 public:
  // 创建一个新群
  bool insertGroup(Group& group);

  // 删除一个群
  bool delGroup();

  // 根据userid查找该用户所加的群
  std::vector<Group> queryGroupById(int userid);

  // 根据groupid查找群信息
  Group queryGroupInfoById(int groupid);
};

#endif  // CHAT_INCLUDE_SERVER_DAO_GROUPMODEL_H