#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <mymuduo/TcpConnection.h>
#include <mymuduo/Timestamp.h>
#include "include/server/dao/FriendModel.h"
#include "include/server/dao/GroupModel.h"
#include "include/server/dao/GroupUserModel.h"
#include "include/server/dao/OfflineMessageModel.h"
#include "include/server/dao/UserModel.h"
#include "include/server/redis/redis.h"
#include "thirdparty/json.h"

#include <functional>
#include <mutex>
#include <unordered_map>

using json = nlohmann::json;

class ChatService {
 public:
  using MsgHandler = std::function<void(const mymuduo::TcpConnectionPtr&, json&,
                                        mymuduo::Timestamp)>;

  static ChatService& getInstance();

  // 登录业务（对应一种消息类型）
  void login(const mymuduo::TcpConnectionPtr& conn, json& js,
             mymuduo::Timestamp time);
  // 注册业务（对应一种消息类型）
  void reg(const mymuduo::TcpConnectionPtr& conn, json& js,
           mymuduo::Timestamp time);
  // 单聊业务（对应一种消息类型）
  void privateChat(const mymuduo::TcpConnectionPtr& conn, json& js,
                   mymuduo::Timestamp time);
  // 群聊业务（对应一种消息类型）
  void groupChat(const mymuduo::TcpConnectionPtr& conn, json& js,
                 mymuduo::Timestamp time);
  // 添加好友请求业务（对应一种消息类型）
  void addFriendReq(const mymuduo::TcpConnectionPtr& conn, json& js,
                    mymuduo::Timestamp time);
  // 添加好友回复业务（对应一种消息类型）
  void addFriendRes(const mymuduo::TcpConnectionPtr& conn, json& js,
                    mymuduo::Timestamp time);
  // 创建群组业务（对应一种消息类型）
  void createGroup(const mymuduo::TcpConnectionPtr& conn, json& js,
                   mymuduo::Timestamp time);
  // 添加群组请求业务（对应一种消息类型）
  void addGroupReq(const mymuduo::TcpConnectionPtr& conn, json& js,
                   mymuduo::Timestamp time);
  // 添加群组回复业务（对应一种消息类型）
  void addGroupRes(const mymuduo::TcpConnectionPtr& conn, json& js,
                   mymuduo::Timestamp time);
  // 注销业务（对应一种消息类型）
  void logOut(const mymuduo::TcpConnectionPtr& conn, json& js,
              mymuduo::Timestamp time);
  // 客户端异常关闭业务（无对应消息类型）
  void clientCloseException(const mymuduo::TcpConnectionPtr& conn);
  // 服务端异常关闭业务（无对应消息类型）
  void reset();

  

  MsgHandler getHandler(int msgId) const;

 private:
  ChatService();
  ~ChatService() = default;
  void handleSubMsg(int userid, std::string message);

  std::unordered_map<int, MsgHandler> msgHandlerMap_;

  // 用户
  UserModel userModel_;
  // 离线消息
  OfflineMessageModel offlineMessageModel_;
  // 好友
  FriendModel friendModel_;
  // 群组
  GroupModel groupModel_;
  // 群组成员
  GroupUserModel groupUserModel_;

  // Redis
  Redis redis_;

  std::mutex connMtx_;
  std::unordered_map<int, mymuduo::TcpConnectionPtr> connectionMap_;
};

#endif  // CHATSERVICE_H