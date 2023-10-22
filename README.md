# ChatServer

ChatServer是一个集群聊天服务器，采用C/C++进行开发：
- 支持登录注册。
- 添加好友。
- 添加群组。
- 单聊。
- 群聊。
- 退出登录。

架构实现：
- 网络层采用MyMuduo实现。
- 利用MySQL存储数据。
- 采用Nginx作为负载均衡。
- 利用Redis发布订阅功能实现跨服务器通信。


## 运行演示




## 编译

### 编译安装MyMuduo



### MySQL

创建数据库表：
- user
- friend
- allgroup
- groupuser
- offlinemessage



### 编译安装 nginx

```bash
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
