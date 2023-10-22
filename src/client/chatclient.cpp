// 聊天客户端
#include "include/public.h"
#include "include/server/dao/Group.h"
#include "include/server/dao/User.h"
#include "thirdparty/json.h"

#include <atomic>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <arpa/inet.h>
#include <semaphore.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

using json = nlohmann::json;

// 当前登录用户
User g_loginUser;

// 当前登录用户的好友列表
std::vector<User> g_currentFriendList;

// 当前登录用户的群组列表
std::vector<Group> g_currentGroupList;

// 当前登录用户的好友请求列表
std::vector<User> g_addFriendReqList;

// 当前登录用户的群组请求列表
std::vector<GroupUser> g_addGroupReqList;

// 用于读写线程同步
sem_t rwsem;

// 控制首页和聊天页面的显示
std::atomic_bool g_isLoginSuccess(false);

// 显示用户信息
void displayUserInfo();

// 显示登录注册页面
void displayMainMenu();

// 显示帮助信息
void help(int fd = 0, const std::string& arg = "");

// 登录业务
void loginService(int clientfd);

// 注册业务
void regService(int clientfd);

// 单聊业务
void privateChatService(int clientfd, const std::string& arg);

// 群聊业务
void groupChatService(int clientf, const std::string& arg);

// 添加好友请求业务
void addFriendReqService(int clientfd, const std::string& arg);

// 添加好友回复业务
void addFriendResService(int clientfd, const std::string& arg);

// 创建群组业务
void createGroupService(int clientfd, const std::string& arg);

// 添加群组请求业务
void addGroupReqService(int clientfd, const std::string& arg);

// 添加群组回复业务
void addGroupResService(int clientfd, const std::string& arg);

// 用户注销业务
void logOutService(int clientfd, const std::string& arg);

// 处理登录响应
void dealWithLoginRes(json& response);

// 处理注册响应
void dealWithRegRes(json& response);

// 读线程函数
void readHandler(int clientfd);

// 获取当前时间
std::string getCurrentTime();

// 支持的命令列表
std::unordered_map<std::string, std::string> commandList = {
    {"help", "显示所有支持的命令，命令格式为：help"},
    {"privateChat", "单聊，命令格式为：privateChat:friendid:message"},
    {"groupChat", "群聊，命令格式为：groupChat:groupid:message"},
    {"addFriendReq", "添加好友请求，命令格式为：addFriendReq:friendid"},
    {"addFriendRes",
     "添加好友回复，命令格式为：addFriendRes:requestNum:yes/no"},
    {"createGroup", "创建群组，命令格式为：createGroup:groupname:groupdesc"},
    {"addGroupReq", "添加群组请求，命令格式为：addGroupReq:groupid"},
    {"addGroupRes", "添加群组回复，命令格式为：addGroupRes:requestNum:yes/no"},
    {"logOut", "退出登录，命令格式为：logOut"}};

// 命令对应的handler
std::unordered_map<std::string, std::function<void(int, std::string)>>
    commandHandler = {{"help", help},
                      {"privateChat", privateChatService},
                      {"groupChat", groupChatService},
                      {"addFriendReq", addFriendReqService},
                      {"addFriendRes", addFriendResService},
                      {"createGroup", createGroupService},
                      {"addGroupReq", addGroupReqService},
                      {"addGroupRes", addGroupResService},
                      {"logOut", logOutService}};

// 显示用户信息
void displayUserInfo() {
  std::cout << "<========================用户信息栏========================>"
            << std::endl;

  // 1. 用户基本信息
  std::cout << "|                                                " << std::endl;
  std::cout << "| userid : " << std::left << std::setw(20)
            << g_loginUser.getId() << "                      " << std::endl;
  std::cout << "| username : " << std::left << std::setw(20)
            << g_loginUser.getName() << "               " << std::endl;
  std::cout << "|                                                " << std::endl;
  // 2. 用户好友信息
  std::cout << "|------------------------Friend List-----------------------|"
            << std::endl;
  if (g_currentFriendList.empty()) {
    std::cout << "|                                                       "
              << std::endl;
    std::cout << "| Empty                                                 "
              << std::endl;
    std::cout << "|                                                       "
              << std::endl;
  } else {
    std::cout << "|                                                       "
              << std::endl;
    for (auto f : g_currentFriendList) {
      std::string id = "[" + std::to_string(f.getId()) + "]";
      std::string name = "<" + f.getName() + ">";
      std::cout << "| " << std::left << std::setw(6) << id << std::left
                << std::setw(15) << name << f.getState() << std::endl;
    }
    std::cout << "|                                                "
              << std::endl;
  }

  // 3. 用户群组信息
  std::cout << "|------------------------Group List------------------------|"
            << std::endl;
  if (g_currentGroupList.empty()) {
    std::cout << "|                                                "
              << std::endl;
    std::cout << "| Empty                                          "
              << std::endl;
    std::cout << "|                                                "
              << std::endl;
  } else {
    std::cout << "|                                                "
              << std::endl;
    for (auto& g : g_currentGroupList) {
      std::string groupid = "[" + std::to_string(g.getGroupid()) + "]";
      std::string groupname = "<" + g.getGroupname() + ">";
      std::cout << "| " << std::left << std::setw(6) << groupid << std::left
                << std::setw(23) << groupname << g.getGroupdesc() << std::endl;
      for (auto& gu : g.getGroupusers()) {
        std::string guid = "[" + std::to_string(gu.getId()) + "]";
        std::string guname = "<" + gu.getName() + ">";
        std::cout << "|  --> " << std::left << std::setw(7) << gu.getRole()
                  << std::left << std::setw(6) << guid << std::left
                  << std::setw(15) << gu.getName() << gu.getState()
                  << std::endl;
      }
    }
    std::cout << "|                                                "
              << std::endl;
  }

  // 4. 添加好友请求消息
  std::cout << "|------------------------FriendReq List--------------------|"
            << std::endl;
  if (g_addFriendReqList.empty()) {
    std::cout << "|                                                "
              << std::endl;
    std::cout << "| Empty                                          "
              << std::endl;
    std::cout << "|                                                "
              << std::endl;
  } else {
    std::cout << "|                                                "
              << std::endl;
    for (auto u : g_addFriendReqList) {
      std::cout << "| userid= " << u.getId() << ", username= " << u.getName()
                << ", 请求添加您为好友！" << std::endl;
    }
    std::cout << "|                                                "
              << std::endl;
  }

  // 5. 添加群组请求消息
  std::cout << "|------------------------GroupReq List---------------------|"
            << std::endl;
  if (g_addGroupReqList.empty()) {
    std::cout << "|                                                "
              << std::endl;
    std::cout << "| Empty                                          "
              << std::endl;
    std::cout << "|                                                "
              << std::endl;
    std::cout << "<========================用户信息栏========================>"
              << std::endl;
  } else {
    std::cout << "|                                                a"
              << std::endl;
    for (auto& gu : g_addGroupReqList) {
      std::cout << "| userid= " << gu.getId() << ", username= " << gu.getName()
                << ", 请求加入" << gu.getGroupid() << "群组！ " << std::endl;
    }
    std::cout << "|                                                "
              << std::endl;
    std::cout << "<========================用户信息栏========================>"
              << std::endl;
  }

  std::cout << "页面已刷新，您有如下新消息 >>>" << std::endl;
}

// 显示首页面
void displayMainMenu() {
  std::cout << "<=====================菜单栏=====================>"
            << std::endl;
  std::cout << "|                                                |"
            << std::endl;
  std::cout << "|                   1. login                     |"
            << std::endl;
  std::cout << "|                   2. register                  |"
            << std::endl;
  std::cout << "|                   3. quit                      |"
            << std::endl;
  std::cout << "|                                                |"
            << std::endl;
  std::cout << "--------------------------------------------------"
            << std::endl;
}

void help(int fd, const std::string& arg) {
  std::cout << std::endl;
  std::cout << "You can execute commands below >>>" << std::endl;
  for (auto item : commandList) {
    std::cout << item.first << " : " << item.second << std::endl;
  }
  std::cout << "<<<" << std::endl;
}

// 登录业务
void loginService(int clientfd) {
  int userid;
  std::string passwd;
  std::cout << "userid: ";
  std::cin >> userid;
  std::cin.get();
  std::cout << "passwd: ";
  getline(std::cin, passwd);

  json js;
  js["msgId"] = MsgType::kLoginMsg;
  js["id"] = userid;
  js["passwd"] = passwd;

  std::string request = js.dump();
  int len = write(clientfd, request.c_str(), strlen(request.c_str()) + 1);
  if (len == -1) {
    std::cerr << "loginService write error" << std::endl;
  }

  // 等待读线程处理响应
  if (sem_wait(&rwsem) == -1) {
    std::cerr << "sem_wait error!" << std::endl;
    exit(1);
  }

  if (g_isLoginSuccess) {
    help();
    while (g_isLoginSuccess) {
      // 1. 获取命令
      std::string cmdLine;
      getline(std::cin, cmdLine);

      // 2. 解析命令
      size_t pos = cmdLine.find(":");
      std::string cmd = cmdLine.substr(0, pos);
      if (commandList.find(cmd) != commandList.end()) {  // 输入命令正确
        commandHandler[cmd](clientfd,
                            cmdLine.substr(pos + 1, std::string::npos));
      } else {
        std::cout << "Unknown command, please check your input!" << std::endl;
      }
    }
  }
}

// 注册业务
void regService(int clientfd) {
  std::string username;
  std::string passwd;
  std::cout << "username: ";
  std::cin >> username;
  std::cout << "passwd: ";
  std::cin >> passwd;

  json js;
  js["msgId"] = 2;
  js["name"] = username;
  js["passwd"] = passwd;

  std::string request = js.dump();
  int len = write(clientfd, request.c_str(), strlen(request.c_str()) + 1);
  if (len == -1) {
  }

  char buf[1024] = {0};
  len = read(clientfd, buf, sizeof(buf));
  if (len == -1) {
  }
}

// 读线程函数
void readHandler(int clientfd) {
  char buf[2048] = {0};
  int byteRead;
  while ((byteRead = read(clientfd, buf, sizeof(buf)))) {  // 不停读取消息
    buf[byteRead] = '\0';
    json msg = json::parse(buf);
    int msgId = msg["msgId"].get<int>();
    if (msgId == MsgType::kPrivateChatMsg) {
      std::cout << msg["time"].get<std::string>();
      std::cout << " [" << msg["id"].get<int>() << "] "
                << " <" << msg["name"].get<std::string>() << "> "
                << "said: " << msg["message"].get<std::string>() << std::endl;
    } else if (msgId == MsgType::kGroupChatMsg) {
      std::cout << "群组" << msg["groupid"].get<int>() << " 有新消息 >>>  ";
      std::cout << msg["time"].get<std::string>();
      std::cout << " [" << msg["id"].get<int>() << "] "
                << " <" << msg["name"].get<std::string>() << "> "
                << "said: " << msg["message"].get<std::string>() << std::endl;
    } else if (msgId == MsgType::kAddFriendReqMsg) {
      User u;
      u.setId(msg["id"].get<int>());
      u.setName(msg["name"]);
      g_addFriendReqList.push_back(u);
      displayUserInfo();
      std::cout << "你有新的添加好友请求！" << std::endl;
    } else if (msgId == MsgType::kAddFriendResMsg) {
      displayUserInfo();
    } else if (msgId == MsgType::kAddGroupReqMsg) {
      GroupUser gu;
      gu.setId(msg["id"].get<int>());
      gu.setName(msg["name"]);
      gu.setGroupid(msg["groupid"].get<int>());
      gu.setRole("normal");
      g_addGroupReqList.push_back(gu);
      displayUserInfo();
      std::cout << "你有新的添加群组请求！" << std::endl;
    } else if (msgId == MsgType::kAddGroupResMsg) {
      std::cout << msg["errmsg"] << std::endl;
    } else if (msgId == MsgType::kLoginAckMsg) {
      dealWithLoginRes(msg);
      sem_post(&rwsem);
    } else if (msgId == MsgType::kRegAckMsg) {
      dealWithRegRes(msg);
      sem_post(&rwsem);
    } else if (msgId == MsgType::kAddFriendReqAckMsg) {
      std::cout << msg["errmsg"] << std::endl;
    } else if (msgId == MsgType::kAddFriendResAckMsg) {
      if (msg["requester"].get<int>() == 1) {
        User u;
        u.setId(msg["friendid"].get<int>());
        u.setName(msg["name"]);
        u.setState("online");

        if (msg["isAgree"] == "yes") {
          g_currentFriendList.push_back(u);
          displayUserInfo();
          std::cout << "[" << u.getId() << "] "
                    << "<" << u.getName() << "> 同意添加你为好友！"
                    << std::endl;
        } else {
          std::cout << "[" << u.getId() << "] "
                    << "<" << u.getName() << "> 拒绝添加你为好友！"
                    << std::endl;
        }
      } else {
        User u;
        u.setId(msg["friendid"].get<int>());
        u.setName(msg["name"]);
        u.setState(msg["state"]);
        g_currentFriendList.push_back(u);
        displayUserInfo();
        std::cout << "新的好友已添加！" << std::endl;
        sem_post(&rwsem);
      }
    } else if (msgId == MsgType::kCreateGroupAckMsg) {
      Group g;
      g.setGroupid(msg["groupid"]);
      g.setGroupname(msg["groupname"]);
      g.setGroupdesc(msg["groupdesc"]);
      GroupUser gu;
      gu.setId(g_loginUser.getId());
      gu.setName(g_loginUser.getName());
      gu.setState(g_loginUser.getState());
      gu.setGroupid(g.getGroupid());
      gu.setRole("creator");
      g.getGroupusers().push_back(gu);
      g_currentGroupList.push_back(g);
      displayUserInfo();
      std::cout << msg["errmsg"].get<std::string>() << std::endl;
      if (msg["errno"].get<int>() == 0) {
        std::cout << "新建群id为：" << g.getGroupid() << "!" << std::endl;
      }
    } else if (msgId == MsgType::kAddGroupReqAckMsg) {
      std::cout << msg["errmsg"].get<std::string>() << std::endl;
    } else if (msgId == MsgType::kAddGroupResAckMsg) {
      GroupUser gu;
      gu.setId(msg["id"]);
      gu.setName(msg["name"]);
      gu.setRole(msg["role"]);
      gu.setState("online");
      gu.setGroupid(msg["groupid"]);

      if (msg["isAgree"] == "yes") {
        Group g;
        std::string groupinfo = msg["groupinfo"];
        json groupjs = json::parse(groupinfo);
        g.setGroupid(groupjs["groupid"].get<int>());
        g.setGroupname(groupjs["groupname"]);
        g.setGroupdesc(groupjs["groupdesc"]);
        std::vector<std::string> usersStr = groupjs["groupusers"];
        for (auto& s : usersStr) {
          json userjs = json::parse(s);
          GroupUser gu;
          gu.setId(userjs["id"].get<int>());
          gu.setName(userjs["name"]);
          gu.setGroupid(groupjs["groupid"].get<int>());
          gu.setState(userjs["state"]);
          gu.setRole(userjs["role"]);
          g.getGroupusers().push_back(gu);
        }

        g_currentGroupList.push_back(g);
        displayUserInfo();
        std::cout << "管理员同意你加入群组" << gu.getGroupid() << "!"
                  << std::endl;
      } else {
        std::cout << "管理员不同意你加入群组" << gu.getGroupid() << "!"
                  << std::endl;
      }
    }
  }
  std::cerr << "server down!" << std::endl;
  exit(1);  // 服务端关闭，客户端也关闭
}

// 单聊业务 arg为friendid:message
void privateChatService(int clientfd, const std::string& arg) {
  size_t pos = arg.find(":");
  if (pos == std::string::npos) {
    std::cout << "Input format error, please check your input!" << std::endl;
    return;
  }
  int friendid = stoi(arg.substr(0, pos));
  std::string message = arg.substr(pos + 1, std::string::npos);

  json js;
  js["msgId"] = MsgType::kPrivateChatMsg;
  js["time"] = getCurrentTime();
  js["id"] = g_loginUser.getId();
  js["name"] = g_loginUser.getName();
  js["to"] = friendid;
  js["message"] = message;

  std::string request = js.dump();
  int len = write(clientfd, request.c_str(), strlen(request.c_str()) + 1);
  if (len == -1) {
    std::cerr << "privateChatService write error" << std::endl;
    exit(1);
  }
}

// 群聊业务 arg为groupid:message
void groupChatService(int clientfd, const std::string& arg) {
  size_t pos = arg.find(":");
  if (pos == std::string::npos) {
    std::cout << "Input format error, please check your input!" << std::endl;
    return;
  }
  int groupid = stoi(arg.substr(0, pos));
  std::string message = arg.substr(pos + 1, std::string::npos);

  json js;
  js["msgId"] = MsgType::kGroupChatMsg;
  js["time"] = getCurrentTime();
  js["id"] = g_loginUser.getId();
  js["name"] = g_loginUser.getName();
  js["groupid"] = groupid;
  js["message"] = message;

  std::string request = js.dump();
  int len = write(clientfd, request.c_str(), strlen(request.c_str()) + 1);
  if (len == -1) {
    std::cerr << "groupChatService write error" << std::endl;
    exit(1);
  }
}

// 添加好友请求业务
void addFriendReqService(int clientfd, const std::string& arg) {
  int friendid = stoi(arg);

  json js;
  js["msgId"] = MsgType::kAddFriendReqMsg;
  js["id"] = g_loginUser.getId();
  js["name"] = g_loginUser.getName();
  js["friendid"] = friendid;

  std::string request = js.dump();

  int len = write(clientfd, request.c_str(), strlen(request.c_str()) + 1);
  if (len == -1) {
    std::cerr << "addFriendReqService write error" << std::endl;
    exit(1);
  }
}

// 添加好友回复业务
void addFriendResService(int clientfd, const std::string& arg) {
  size_t pos = arg.find(":");
  if (pos == std::string::npos) {
    std::cout << "Input format error, please check your input!" << std::endl;
    return;
  }
  int requestNum = stoi(arg.substr(0, pos));
  std::string isAgree = arg.substr(pos + 1, std::string::npos);

  User u = g_addFriendReqList[requestNum];
  json js;
  js["msgId"] = MsgType::kAddFriendResMsg;
  js["id"] = u.getId();
  js["friendid"] = g_loginUser.getId();
  js["name"] = g_loginUser.getName();
  js["isAgree"] = isAgree;
  g_addFriendReqList.erase(g_addFriendReqList.begin() + requestNum);

  std::string request = js.dump();
  int len = write(clientfd, request.c_str(), strlen(request.c_str()) + 1);
  if (len == -1) {
    std::cerr << "addFriendResService write error" << std::endl;
    exit(1);
  }

  sem_wait(&rwsem);
}

// 创建群组业务 arg为groupname:groupdesc
void createGroupService(int clientfd, const std::string& arg) {
  size_t pos = arg.find(":");
  if (pos == std::string::npos) {
    std::cout << "Input format error, please check your input!" << std::endl;
    return;
  }
  std::string groupname = arg.substr(0, pos);
  std::string groupdesc = arg.substr(pos + 1, std::string::npos);

  json js;
  js["msgId"] = MsgType::kCreateGroupMsg;
  js["userid"] = g_loginUser.getId();
  js["groupname"] = groupname;
  js["groupdesc"] = groupdesc;

  std::string request = js.dump();

  int len = write(clientfd, request.c_str(), strlen(request.c_str()) + 1);
  if (len == -1) {
    std::cerr << "createGroupService write error" << std::endl;
    exit(1);
  }
}

// 添加群组请求业务 arg为groupid
void addGroupReqService(int clientfd, const std::string& arg) {
  int groupid = stoi(arg);

  json js;
  js["msgId"] = MsgType::kAddGroupReqMsg;
  js["id"] = g_loginUser.getId();
  js["name"] = g_loginUser.getName();
  js["groupid"] = groupid;

  std::string request = js.dump();
  int len = write(clientfd, request.c_str(), strlen(request.c_str()) + 1);
  if (len == -1) {
    std::cerr << "addGroupReqService write error!" << std::endl;
    exit(1);
  }
}

// 添加群组回复业务 arg为requestNum:isAgree
void addGroupResService(int clientfd, const std::string& arg) {
  size_t pos = arg.find(":");
  if (pos == std::string::npos) {
    std::cout << "Input format error, please check your input!" << std::endl;
    return;
  }
  int requestNum = stoi(arg.substr(0, pos));
  std::string isAgree = arg.substr(pos + 1, std::string::npos);

  GroupUser gu = g_addGroupReqList[requestNum];
  json js;
  js["msgId"] = MsgType::kAddGroupResMsg;
  js["id"] = gu.getId();
  js["name"] = gu.getName();
  js["groupid"] = gu.getGroupid();
  js["isAgree"] = isAgree;
  g_addGroupReqList.erase(g_addGroupReqList.begin() + requestNum);

  std::string request = js.dump();
  int len = write(clientfd, request.c_str(), strlen(request.c_str()) + 1);
  if (len == -1) {
    std::cerr << "addGroupResService write error!" << std::endl;
    exit(1);
  }

  if (isAgree == "yes") {
    for (auto& g : g_currentGroupList) {
      if (g.getGroupid() == gu.getGroupid()) {
        g.getGroupusers().push_back(gu);
        break;
      }
    }
    displayUserInfo();
    std::cout << "新的成员添加到群组" << gu.getGroupid() << "!" << std::endl;
  }
}

void logOutService(int clientfd, const std::string& arg) {
  json js;
  js["msgId"] = MsgType::kLogOutMsg;
  js["id"] = g_loginUser.getId();

  std::string request = js.dump();
  int len = write(clientfd, request.c_str(), strlen(request.c_str()) + 1);
  if (len == -1) {
    std::cerr << "logOutService write error!" << std::endl;
    exit(1);
  }

  g_isLoginSuccess = false;
}

// 处理登录响应
void dealWithLoginRes(json& response) {
  if (response["errno"].get<int>() != 0) {  // 登录失败
    std::cout << response["errmsg"].get<std::string>() << std::endl;
    g_isLoginSuccess = false;
  } else {  // 登录成功
    g_loginUser.setId(response["id"].get<int>());
    g_loginUser.setName(response["name"]);
    g_loginUser.setState(response["state"]);

    // 获取好友列表
    g_currentFriendList.clear();
    if (response.contains("friends")) {
      std::vector<std::string> vec = response["friends"];
      for (auto& s : vec) {
        json js = json::parse(s);
        User u;
        u.setId(js["id"].get<int>());
        u.setName(js["name"]);
        u.setState(js["state"]);
        g_currentFriendList.push_back(u);
      }
    }

    // 获取群组消息
    g_currentGroupList.clear();
    if (response.contains("groups")) {
      std::vector<std::string> vec = response["groups"];
      for (auto& s : vec) {
        json groupjs = json::parse(s);
        Group g;
        g.setGroupid(groupjs["groupid"].get<int>());
        g.setGroupname(groupjs["groupname"]);
        g.setGroupdesc(groupjs["groupdesc"]);

        std::vector<std::string> vec2 = groupjs["groupusers"];
        for (const auto& us : vec2) {
          json userjs = json::parse(us);
          GroupUser gu;
          gu.setId(userjs["userid"].get<int>());
          gu.setName(userjs["name"]);
          gu.setState(userjs["state"]);
          gu.setRole(userjs["role"]);
          g.getGroupusers().push_back(gu);
        }
        g_currentGroupList.push_back(g);
      }
    }

    // 获取离线消息
    std::vector<std::string> offmsgs;
    if (response.contains("offlinemessage")) {
      std::vector<std::string> vec = response["offlinemessage"];
      for (auto& s : vec) {
        json msg = json::parse(s);
        int msgId = msg["msgId"].get<int>();
        if (msgId == MsgType::kPrivateChatMsg) {
          char offmsg[128] = {0};
          sprintf(offmsg, "%s [%d] <%s> said: %s\n",
                  msg["time"].get<std::string>().c_str(), msg["id"].get<int>(),
                  msg["name"].get<std::string>().c_str(),
                  msg["message"].get<std::string>().c_str());
          offmsgs.push_back(offmsg);
        } else if (msgId == MsgType::kGroupChatMsg) {
          char offmsg[128] = {0};
          sprintf(offmsg, "群组[%d]新消息 >>> %s [%d] <%s> said: %s\n",
                  msg["groupid"].get<int>(),
                  msg["time"].get<std::string>().c_str(), msg["id"].get<int>(),
                  msg["name"].get<std::string>().c_str(),
                  msg["message"].get<std::string>().c_str());
          offmsgs.push_back(offmsg);
        } else if (msgId == MsgType::kAddFriendReqMsg) {
          User u;
          u.setId(msg["id"].get<int>());
          u.setName(msg["name"]);
          g_addFriendReqList.push_back(u);
        } else if (MsgType::kAddFriendResMsg) {
        } else if (MsgType::kAddGroupReqMsg) {
          GroupUser gu;
          gu.setId(msg["id"].get<int>());
          gu.setName(msg["name"]);
          gu.setGroupid(msg["groupid"].get<int>());
          gu.setRole("normal");
          g_addGroupReqList.push_back(gu);
        } else if (msgId == MsgType::kAddGroupResMsg) {
        }
      }
    }

    displayUserInfo();
    std::cout << "登录成功！" << std::endl;
    for (auto& s : offmsgs) {
      std::cout << s << std::endl;
    }

    g_isLoginSuccess = true;
  }
}

// 处理注册响应
void dealWithRegRes(json& response) {
  std::cout << response["errmsg"].get<std::string>() << std::endl;
  if (response["errno"] == 0) {
    std::cout << "您的id为：" << response["id"] << "， 请牢记！" << std::endl;
  }
}

std::string getCurrentTime() {
  time_t t = time(nullptr);
  std::string timestr = ctime(&t);
  return timestr.substr(0, timestr.size() - 1);
}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    std::cout << "Usage: " << argv[0] << " <ip> <port>" << std::endl;
    exit(1);
  }

  // 建立socket连接
  int clientfd = socket(AF_INET, SOCK_STREAM, 0);
  if (clientfd == -1) {
    std::cerr << "socket() error!" << std::endl;
    exit(1);
  }
  struct sockaddr_in serverAddr;
  memset(&serverAddr, sizeof(serverAddr), 0);
  serverAddr.sin_family = AF_INET;
  inet_pton(AF_INET, argv[1], &serverAddr.sin_addr);
  serverAddr.sin_port = htons(atoi(argv[2]));

  int ret =
      connect(clientfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
  if (ret == -1) {
    std::cerr << "connect() error!" << std::endl;
    close(clientfd);
    exit(1);
  }

  sem_init(&rwsem, 0, 0);

  // 连接成功，开启读线程
  std::thread t(readHandler, clientfd);
  t.detach();

  displayMainMenu();
  int cmd = -1;
  while (true) {
    std::cout << "choice below: " << std::endl;
    std::cin >> cmd;
    switch (cmd) {
      case 1:  // 登录
        loginService(clientfd);
        displayMainMenu();
        break;
      case 2:  // 注册
        regService(clientfd);
        displayMainMenu();
        break;
      case 3:
        exit(0);
        break;
      default:
        break;
    }
  }
}