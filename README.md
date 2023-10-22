# ChatServer

ChatServer是一个集群聊天服务器，采用C/C++进行开发，支持如下功能：
- 登录以及注册。
- 添加好友。
- 添加群组。
- 单聊。
- 群聊。
- 退出登录。


- 网络层采用MyMuduo实现。
- 利用MySQL存储数据。
- 采用Nginx作为负载均衡。
- 利用Redis发布订阅功能实现跨服务器通信。


## 运行演示




## 编译运行

### 编译安装MyMuduo

集群聊天服务器的网络部分利用自己之前使用C++11重写的MyMuduo项目进行实现。

编译安装过程参考：

### MySQL

创建数据库表：
- user：用户表
- friend：用户好友表
- allgroup：群组表
- groupuser：群组成员表
- offlinemessage：离线消息表



### 编译安装 nginx

```bash
# 0. 安装依赖
# 包括gcc、make、pcre、openssl、zlib等

# 1. 下载 nginx 源码
wget http://nginx.org/download/nginx-1.22.1.tar.gz

# 2. 解压
tar -zxvf nginx-1.22.1.tar.gz && cd nginx-1.22.1.tar.gz

# 3. 配置
./configure --with-stream

# 4. 编译安装
make && sudo make install

# 5. 启动 nginx 服务
cd /usr/local/nginx/sbin/nginx && ./nginx

# 6. 修改配置文件
sudo vim /usr/local/nginx/conf/nginx.conf

    # 在文件中添加集群服务器配置
    # 以下以两台服务器为例：
    # 
    # ...
    # 
    # stream {
    
    #     upstream MyServer {
    #         server 127.0.0.1:8001 weight=1 max_fails=3 fail_timeout=30s;  # 实际工作服务器监听8001
    #         server 127.0.0.1:8002 weight=1 max_fails=3 fail_timeout=30s;  # 实际工作服务器监听8002
    #     }
    
    #     server {
    #         proxy_connect_timeout 1s;
    #         listen 8000;  # 反向代理服务器监听8000端口
    #         proxy_pass MyServer;
    #         tcp_nodelay on;
    #     }

    # }
    # 
    # ...
    # 

# 7. 使配置生效
./nginx -s reload
```

### 安装 redis-server

```bash
# ubuntu系统

# 1. 安装 Redis 服务器
sudo apt-get update
sudo apt-get install redis-server

# 2. 启动 Redis
redis-server
```
