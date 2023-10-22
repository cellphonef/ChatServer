#ifndef CHAT_INCLUDE_SERVER_DAO_FRIEND_H
#define CHAT_INCLUDE_SERVER_DAO_FRIEND_H

class Friend {
 public:
  Friend() = default;
  ~Friend() = default;

  int getUserId() const { return userid_; }

  int getFriendId() const { return friendid_; }

  void setUserId(int userid) { userid_ = userid; }
  void setFriendId(int friendid) { friendid_ = friendid; }

 private:
  int userid_;
  int friendid_;
};

#endif  // CHAT_INCLUDE_SERVER_DAO_FRIEND_H