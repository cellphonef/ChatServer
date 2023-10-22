#ifndef CHAT_INCLUDE_SERVER_DAO_OFFLINEMESSAGEMODEL_H
#define CHAT_INCLUDE_SERVER_DAO_OFFLINEMESSAGEMODEL_H

#include "include/server/dao/OfflineMessage.h"

#include <string>
#include <vector>

class OfflineMessageModel {
 public:
  bool insertMessage(OfflineMessage message);
  bool delMessage(OfflineMessage message);
  std::vector<std::string> queryMessage(int id);
};

#endif  // CHAT_INCLUDE_SERVER_DAO_OFFLINEMESSAGEMODEL_H