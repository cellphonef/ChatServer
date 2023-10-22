#ifndef CHAT_INCLUDE_SERVER_DAO_OFFLINEMESSAGE_H
#define CHAT_INCLUDE_SERVER_DAO_OFFLINEMESSAGE_H

#include <string>

class OfflineMessage {
 public:
  OfflineMessage() = default;
  ~OfflineMessage() = default;
  void setId(int id) { id_ = id; }
  void setMessage(const std::string& message) { message_ = message; }
  int getId() const { return id_; }
  std::string getMessage() const { return message_; }

 private:
  int id_;
  std::string message_;
};

#endif  // CHAT_INCLUDE_SERVER_DAO_OFFLINEMESSAGE_