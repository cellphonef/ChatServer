#include "include/server/chatservice.h"
#include <mymuduo/Logging.h>
#include "include/public.h"

#include <functional>
#include <string>
#include <vector>

using namespace std::placeholders;

ChatService& ChatService::getInstance() {
  static ChatService service;
  return service;
}

ChatService::ChatService() {
  msgHandlerMap_[MsgType::kLoginMsg] =
      std::bind(&ChatService::login, this, _1, _2, _3);
  msgHandlerMap_[MsgType::kRegMsg] =
      std::bind(&ChatService::reg, this, _1, _2, _3);
  msgHandlerMap_[MsgType::kPrivateChatMsg] =
      std::bind(&ChatService::privateChat, this, _1, _2, _3);
  msgHandlerMap_[MsgType::kGroupChatMsg] =
      std::bind(&ChatService::groupChat, this, _1, _2, _3);
  msgHandlerMap_[MsgType::kAddFriendReqMsg] =
      std::bind(&ChatService::addFriendReq, this, _1, _2, _3);
  msgHandlerMap_[MsgType::kAddFriendResMsg] =
      std::bind(&ChatService::addFriendRes, this, _1, _2, _3);
  msgHandlerMap_[MsgType::kCreateGroupMsg] =
      std::bind(&ChatService::createGroup, this, _1, _2, _3);
  msgHandlerMap_[MsgType::kAddGroupReqMsg] =
      std::bind(&ChatService::addGroupReq, this, _1, _2, _3);
  msgHandlerMap_[MsgType::kAddGroupResMsg] =
      std::bind(&ChatService::addGroupRes, this, _1, _2, _3);
  msgHandlerMap_[MsgType::kLogOutMsg] =
      std::bind(&ChatService::logOut, this, _1, _2, _3);

  if (redis_.connect()) {
    redis_.setNotifyHandler(
        std::bind(&ChatService::handleSubMsg, this, _1, _2));
  }
}

void ChatService::handleSubMsg(int userid, std::string message) {
  LOG_INFO << "DO HANDLESUBMSG SERVICE";
  {
    std::lock_guard<std::mutex> guard(connMtx_);
    auto it = connectionMap_.find(userid);
    if (it != connectionMap_.end()) {  // 在线，转发
      it->second->send(message);
      return;
    }
  }
  // 不在线，存储离线消息
  OfflineMessage offmsg;
  offmsg.setId(userid);
  offmsg.setMessage(message);
  offlineMessageModel_.insertMessage(offmsg);
}

void ChatService::login(const mymuduo::TcpConnectionPtr& conn, json& js,
                        mymuduo::Timestamp time) {
  int id = js["id"].get<int>();
  std::string passwd = js["passwd"];

  User user = userModel_.queryUserById(id);

  if (user.getId() == id && user.getPasswd() == passwd) {  // 账号密码正确
    if (user.getState() == "online") {                     // 已经登录
      json response;
      response["msgId"] = MsgType::kLoginAckMsg;
      response["errno"] = 2;
      response["errmsg"] = "该账号已经登录，请勿重复登录！";
      conn->send(response.dump());
    } else {  // 还未登录
      // 1. 更新数据库为登录状态
      user.setState("online");
      userModel_.updateState(user);

      // 2. 保存该连接（临界区）
      {
        std::lock_guard<std::mutex> guard(connMtx_);
        connectionMap_[user.getId()] = conn;
      }

      redis_.subscribe(user.getId());

      json response;
      // 3. 离线消息处理
      OfflineMessage offmsg;
      offmsg.setId(id);
      std::vector<std::string> messages = offlineMessageModel_.queryMessage(id);
      if (!messages.empty()) {
        response["offlinemessage"] = messages;
        offlineMessageModel_.delMessage(offmsg);
      }

      // 4. 返回好友列表
      std::vector<User> friends = friendModel_.queryFriendById(id);
      if (!friends.empty()) {
        std::vector<std::string> res;
        for (auto u : friends) {
          json js;
          js["id"] = u.getId();
          js["name"] = u.getName();
          js["state"] = u.getState();
          res.push_back(js.dump());
        }
        response["friends"] = res;
      }

      // 5. 返回群组以及对应的群成员列表
      std::vector<Group> groups =
          groupModel_.queryGroupById(id);  // 获得id所属的所有群组

      if (!groups.empty()) {
        std::vector<std::string> vec;

        // [{"groupid":x, "groupname":xxx,
        //   "groupdesc":xxx, groupusers:[...]}, ...]
        json group;
        for (const auto& g : groups) {
          group["groupid"] = g.getGroupid();
          group["groupname"] = g.getGroupname();
          group["groupdesc"] = g.getGroupdesc();
          std::vector<std::string> vec2;  // 存放user信息
          std::vector<GroupUser> groupusers =
              groupUserModel_.queryGroupUserById(g.getGroupid());
          for (const auto& gu : groupusers) {
            json js;
            js["userid"] = gu.getId();
            js["name"] = gu.getName();
            js["state"] = gu.getState();
            js["role"] = gu.getRole();

            vec2.push_back(js.dump());
          }
          group["groupusers"] = vec2;
          vec.push_back(group.dump());
        }
        response["groups"] = vec;
      }

      // 6. 返回登录消息
      response["msgId"] = MsgType::kLoginAckMsg;
      response["id"] = id;
      response["name"] = user.getName();
      response["state"] = user.getState();
      response["errno"] = 0;
      response["errmsg"] = "登录成功！";

      conn->send(response.dump());
    }

  } else {  // 查询失败，用户名或者密码错误
    json response;
    response["msgId"] = MsgType::kLoginAckMsg;
    response["errno"] = 1;
    response["errmsg"] = "登录失败，用户名或密码错误！";
    conn->send(response.dump());
  }

  LOG_INFO << "DO LOGIN SERVICE";
}

void ChatService::reg(const mymuduo::TcpConnectionPtr& conn, json& js,
                      mymuduo::Timestamp time) {
  std::string name = js["name"];
  std::string passwd = js["passwd"];

  User user;
  user.setName(name);
  user.setPasswd(passwd);

  if (userModel_.insertUser(user)) {
    json response;
    response["msgId"] = MsgType::kRegAckMsg;
    response["errno"] = 0;
    response["errmsg"] = "注册成功！";
    response["id"] = user.getId();
    conn->send(response.dump());
  } else {
    json response;
    response["msgId"] = MsgType::kRegAckMsg;
    response["errno"] = 1;
    response["errmsg"] = "用户名已存在，注册失败！";
    conn->send(response.dump());
  }

  LOG_INFO << "DO REG SERVICE";
}

// 单聊业务
void ChatService::privateChat(const mymuduo::TcpConnectionPtr& conn, json& js,
                              mymuduo::Timestamp time) {
  LOG_INFO << "DO PRIVATECHAT SERVICE";

  int toId = js["to"].get<int>();  // 接收方

  std::lock_guard<std::mutex> guard(connMtx_);
  auto it = connectionMap_.find(toId);
  if (it != connectionMap_.end()) {  // 在线，转发
    it->second->send(js.dump());
    return;
  }

  User u = userModel_.queryUserById(toId);
  if (u.getState() == "online") {
    redis_.publish(toId, js.dump());
    return;
  }

  // 不在线，存储离线消息
  OfflineMessage message;
  message.setId(toId);
  message.setMessage(js.dump());
  offlineMessageModel_.insertMessage(message);
}
// 群聊业务
void ChatService::groupChat(const mymuduo::TcpConnectionPtr& conn, json& js,
                            mymuduo::Timestamp time) {
  LOG_INFO << "DO GROUPCHAT SERVICE";

  int userid = js["id"].get<int>();
  int groupid = js["groupid"].get<int>();

  std::vector<GroupUser> groupusers =
      groupUserModel_.queryGroupUserById(groupid);

  std::lock_guard<std::mutex> guard(connMtx_);
  for (auto& gu : groupusers) {
    if (gu.getId() == userid) continue;  //不转发给自己

    // 不是自己，则判断是否在线，在线则转发
    auto it = connectionMap_.find(gu.getId());
    if (it != connectionMap_.end()) {
      it->second->send(js.dump());
      continue;
    }

    User u = userModel_.queryUserById(gu.getId());
    if (u.getState() == "online") {
      redis_.publish(gu.getId(), js.dump());
      continue;
    }

    // 不在线，存储离线消息
    OfflineMessage message;
    message.setId(gu.getId());
    message.setMessage(js.dump());
    offlineMessageModel_.insertMessage(message);
  }
}

// 添加好友请求业务
void ChatService::addFriendReq(const mymuduo::TcpConnectionPtr& conn, json& js,
                               mymuduo::Timestamp time) {
  LOG_INFO << "DO ADDFRINEDREQ SERVICE";
  int friendid = js["friendid"].get<int>();

  User u = userModel_.queryUserById(friendid);
  if (u.getId() == -1) {
    json response;
    response["msgId"] = MsgType::kAddFriendReqAckMsg;
    response["errno"] = 1;
    response["errmsg"] = "你添加的用户不存在！";
    conn->send(response.dump());
    return;
  }

  {
    std::lock_guard<std::mutex> guard(connMtx_);
    auto it = connectionMap_.find(friendid);
    if (it != connectionMap_.end()) {  // 在线，转发添加好友消息
      it->second->send(js.dump());
      json response;
      response["msgId"] = MsgType::kAddFriendReqAckMsg;
      response["errno"] = 0;
      response["errmsg"] = "添加好友请求发送成功，等待对方通过！";
      conn->send(response.dump());
      return;
    }
  }

  if (u.getState() == "online") {
    redis_.publish(u.getId(), js.dump());
    return;
  }

  // 不在线，存储离线消息
  OfflineMessage message;
  message.setId(u.getId());
  message.setMessage(js.dump());
  offlineMessageModel_.insertMessage(message);
}

// 添加好友回复业务
void ChatService::addFriendRes(const mymuduo::TcpConnectionPtr& conn, json& js,
                               mymuduo::Timestamp time) {
  int userid = js["id"].get<int>();
  int friendid = js["friendid"].get<int>();
  std::string isAgree = js["isAgree"];

  json responseToSelf;
  json responseToFriend;

  responseToFriend["msgId"] = MsgType::kAddFriendResAckMsg;
  responseToFriend["friendid"] = friendid;
  responseToFriend["name"] = js["name"];
  responseToFriend["errno"] = 0;
  responseToFriend["isAgree"] = js["isAgree"];
  responseToFriend["requester"] = 1;
  if (isAgree == "yes") {
    Friend f;
    // 双向添加
    f.setUserId(userid);
    f.setFriendId(friendid);
    friendModel_.insertFriend(f);
    f.setUserId(friendid);
    f.setFriendId(userid);
    friendModel_.insertFriend(f);

    User u = userModel_.queryUserById(userid);
    responseToSelf["msgId"] = MsgType::kAddFriendResAckMsg;
    responseToSelf["requester"] = 0;
    responseToSelf["friendid"] = u.getId();
    responseToSelf["name"] = u.getName();
    responseToSelf["state"] = u.getState();
    responseToSelf["errno"] = 0;
    conn->send(responseToSelf.dump());
  }

  {
    std::lock_guard<std::mutex> guard(connMtx_);
    auto it = connectionMap_.find(userid);
    if (it != connectionMap_.end()) {  // 在线，转发添加好友回复消息
      it->second->send(responseToFriend.dump());
      return;
    }
  }

  User u = userModel_.queryUserById(userid);
  if (u.getState() == "online") {
    redis_.publish(u.getId(), responseToFriend.dump());
    return;
  }

  // 不在线，存储离线消息
  OfflineMessage message;
  message.setId(userid);
  message.setMessage(responseToFriend.dump());
  offlineMessageModel_.insertMessage(message);
}

// 创建群组业务（对应一种消息类型）
void ChatService::createGroup(const mymuduo::TcpConnectionPtr& conn, json& js,
                              mymuduo::Timestamp time) {
  LOG_INFO << "DO CREATEGROUP SERVICE";
  int userid = js["userid"].get<int>();
  std::string groupName = js["groupname"];
  std::string groupDesc = js["groupdesc"];

  Group g;
  g.setGroupname(groupName);
  g.setGroupdesc(groupDesc);

  json response;
  response["msgId"] = MsgType::kCreateGroupAckMsg;
  if (groupModel_.insertGroup(g)) {
    GroupUser gu;
    gu.setId(userid);
    gu.setGroupid(g.getGroupid());
    gu.setRole("creator");
    groupUserModel_.insertGroupUser(gu);

    response["errno"] = 0;
    response["groupid"] = g.getGroupid();
    response["groupname"] = g.getGroupname();
    response["groupdesc"] = g.getGroupdesc();
    response["errmsg"] = "创建群组成功！";
  } else {
    response["errno"] = 1;
    response["errmsg"] = "创建群组失败！";
  }
  conn->send(response.dump());
}

// 添加群组请求业务（对应一种消息类型）
void ChatService::addGroupReq(const mymuduo::TcpConnectionPtr& conn, json& js,
                              mymuduo::Timestamp time) {
  LOG_INFO << "DO ADDGROUPREQ SERVICE";
  int userid = js["id"].get<int>();
  int groupid = js["groupid"].get<int>();

  // 根据groupid找到管理员
  int groupCreator = groupUserModel_.queryCreatorById(groupid);

  json response;
  bool flag = false;
  {
    std::lock_guard<std::mutex> guard(connMtx_);
    auto it = connectionMap_.find(groupCreator);
    if (it != connectionMap_.end()) {  // 在线，转发添加群组请求消息
      it->second->send(js.dump());
      flag = true;
    }
  }

  User u = userModel_.queryUserById(groupCreator);
  if (u.getState() == "online") {
    redis_.publish(u.getId(), js.dump());
  } else if (flag == true) {  // 不在线，存储离线消息
    OfflineMessage message;
    message.setId(userid);
    message.setMessage(js.dump());
    offlineMessageModel_.insertMessage(message);
  }

  response["msgId"] = MsgType::kAddGroupReqAckMsg;
  response["errno"] = 0;
  response["errmsg"] = "添加群组请求发送成功，等待管理员通过！";
  conn->send(response.dump());
}

void ChatService::addGroupRes(const mymuduo::TcpConnectionPtr& conn, json& js,
                              mymuduo::Timestamp time) {
  LOG_INFO << "DO ADDGROUPRES SERVICE";

  json response;
  int userid = js["id"].get<int>();
  response["msgId"] = MsgType::kAddGroupResAckMsg;
  response["isAgree"] = js["isAgree"];

  GroupUser gu;
  gu.setId(userid);
  gu.setGroupid(js["groupid"].get<int>());
  gu.setRole("normal");

  response["groupid"] = gu.getGroupid();
  response["id"] = gu.getId();
  response["role"] = gu.getRole();
  response["name"] = gu.getName();
  response["errno"] = 0;

  if (js["isAgree"] == "yes") {
    groupUserModel_.insertGroupUser(gu);
    Group g = groupModel_.queryGroupInfoById(gu.getGroupid());
    json groupjs;
    groupjs["groupid"] = g.getGroupid();
    groupjs["groupname"] = g.getGroupname();
    groupjs["groupdesc"] = g.getGroupdesc();
    std::vector<std::string> vec;
    for (auto& gu : g.getGroupusers()) {
      json gujs;
      gujs["id"] = gu.getId();
      gujs["name"] = gu.getName();
      gujs["state"] = gu.getState();
      gujs["role"] = gu.getRole();
      vec.push_back(gujs.dump());
    }
    groupjs["groupusers"] = vec;
    response["groupinfo"] = groupjs.dump();
  }

  {
    std::lock_guard<std::mutex> guard(connMtx_);
    auto it = connectionMap_.find(userid);
    if (it != connectionMap_.end()) {
      it->second->send(response.dump());
      return;
    }
  }

  User u = userModel_.queryUserById(userid);
  if (u.getState() == "online") {
    redis_.publish(userid, response.dump());
    return;
  }

  // 不在线，存储离线消息
  OfflineMessage message;
  message.setId(userid);
  message.setMessage(response.dump());
  offlineMessageModel_.insertMessage(message);
}

// 注销业务（对应一种消息类型）
void ChatService::logOut(const mymuduo::TcpConnectionPtr& conn, json& js,
                         mymuduo::Timestamp time) {
  LOG_INFO << "DO LOGOUT SERVICE";
  int userid = js["id"].get<int>();

  {
    std::lock_guard<std::mutex> guard(connMtx_);
    auto it = connectionMap_.find(userid);
    if (it != connectionMap_.end()) {
      connectionMap_.erase(it);
    }
  }

  redis_.unsubscribe(userid);

  User u;
  u.setId(userid);
  u.setState("offline");
  userModel_.updateState(u);
}

// 客户端异常关闭业务
void ChatService::clientCloseException(const mymuduo::TcpConnectionPtr& conn) {
  User u;
  {
    std::lock_guard<std::mutex> guard(connMtx_);
    for (auto it = connectionMap_.begin(); it != connectionMap_.end(); ++it) {
      if (it->second.get() == conn.get()) {
        u.setId(it->first);
        u.setState("offline");
        connectionMap_.erase(it);
        break;
      }
    }
  }

  if (u.getId() != -1) {
    redis_.unsubscribe(u.getId());
    userModel_.updateState(u);
  }

  LOG_INFO << "DO CLIENTCLOSEEXCETION SERVICE";
}

// 服务端接收到SIGINT关闭
void ChatService::reset() {
  {
    std::lock_guard<std::mutex> guard(connMtx_);
    for (auto it = connectionMap_.begin(); it != connectionMap_.end(); ++it) {
      redis_.unsubscribe(it->first);
    }
  }

  userModel_.resetState();
  LOG_INFO << "DO RESET SERVICE";
}

ChatService::MsgHandler ChatService::getHandler(int msgId) const {
  auto it = msgHandlerMap_.find(msgId);
  if (it ==
      msgHandlerMap_.end()) {  // 查找不到该业务，则返回一个默认的处理器，空操作
    return [=](const mymuduo::TcpConnectionPtr& conn, json& js,
               mymuduo::Timestamp time) {
      LOG_ERROR << "mgsId=" << msgId << " corresponds service can not found!";
    };
  }
  return it->second;
}