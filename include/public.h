#ifndef CHAT_INCLUDE_PUBLIC_H
#define CHAT_INCLUDE_PUBLIC_H

// 消息类型
enum MsgType {
  // 请求部分
  kLoginMsg = 1,        // 登录消息类型
  kRegMsg,              // 注册消息类型
  kPrivateChatMsg,      // 单聊消息类型
  kGroupChatMsg,        // 群聊消息类型
  kAddFriendReqMsg,     // 添加好友请求消息类型
  kAddFriendResMsg,     // 添加好友回复消息类型
  kCreateGroupMsg,      // 创建群组消息类型
  kAddGroupReqMsg,      // 添加群组请求消息类型
  kAddGroupResMsg,      // 添加群组回复消息类型
  kLogOutMsg,           // 注销功能

  // 响应部分
  kLoginAckMsg,         // 登录响应消息类型
  kRegAckMsg,           // 注册响应消息类型
  kAddFriendReqAckMsg,  // 添加好友请求响应消息类型
  kAddFriendResAckMsg,  // 添加好友回复响应消息类型
  kCreateGroupAckMsg,   // 创建群组响应消息类型
  kAddGroupReqAckMsg,   // 添加群组请求响应消息类型
  kAddGroupResAckMsg    // 添加群组回复响应消息类型
};

#endif  // CHAT_INCLUDE_PUBLIC_H