#ifndef CHAT_INCLUDE_DAO_USER_H
#define CHAT_INCLUDE_DAO_USER_H

#include <string>

class User {
 public:
  User() : id_(-1), state_("offline") {}
  ~User() = default;

  int getId() const { return id_; }
  std::string getName() const { return name_; }
  std::string getPasswd() const { return passwd_; }
  std::string getState() const { return state_; }

  void setId(int id) { id_ = id; }
  void setName(std::string name) { name_ = name; }
  void setPasswd(std::string passwd) { passwd_ = passwd; }
  void setState(std::string state) { state_ = state; }

 private:
  int id_;
  std::string name_;
  std::string passwd_;
  std::string state_;
};

#endif  // CHAT_INCLUDE_DAO_USER_H