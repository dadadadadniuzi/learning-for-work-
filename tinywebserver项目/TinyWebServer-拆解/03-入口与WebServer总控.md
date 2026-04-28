---
title: 入口与WebServer总控
tags:
  - project
  - interview
  - webserver
---

# 入口与WebServer总控

返回：[[TinyWebServer-面试拆解笔记]]

相关：[[TinyWebServer-拆解/02-请求链路与从零实现]]、[[TinyWebServer-拆解/04-http_conn与HTTP状态机]]、[[TinyWebServer-拆解/06-定时器与连接管理]]

## `main.cpp` 在做什么

主流程很清晰：

1. 设置数据库用户名、密码、库名
2. 创建 `Config`，解析命令行参数
3. 创建 `WebServer`
4. 调用 `init()`
5. 初始化日志
6. 初始化数据库连接池
7. 初始化线程池
8. 设置触发模式
9. 建立监听和 epoll
10. 进入事件循环

一句话理解：

`main` 就是整个服务器的装配入口。

## `Config` 模块负责什么

`Config` 负责解析命令行参数，决定服务器的运行方式。

### `Config::Config()`

- 作用：设置默认配置
- 典型默认值：
  - 端口 `9006`
  - 默认同步日志
  - 默认 `LT + LT`
  - 默认 Proactor
  - 线程数 `8`
  - 数据库连接数 `8`

### `Config::parse_arg(int argc, char *argv[])`

- 作用：解析命令行参数
- 常见参数：
  - `-p`：端口
  - `-l`：日志方式
  - `-m`：LT/ET 组合
  - `-o`：优雅关闭
  - `-s`：数据库连接数
  - `-t`：线程数
  - `-c`：是否关闭日志
  - `-a`：并发模型

## `WebServer` 是什么角色

`WebServer` 是整个项目的大脑。

它负责：

- 网络监听
- epoll 创建和事件循环
- 连接对象管理
- 定时器调度
- 线程池和数据库连接池初始化
- 日志初始化
- 信号处理

## `WebServer` 重要成员

- `m_port`
  监听端口
- `m_root`
  网站根目录
- `m_epollfd`
  epoll 实例
- `m_listenfd`
  监听 socket
- `users`
  `http_conn` 对象数组
- `users_timer`
  定时器相关数据数组
- `m_pool`
  线程池
- `m_connPool`
  数据库连接池
- `m_pipefd`
  信号管道

## 为什么 `users[fd]` 这种数组方式好用

因为 socket fd 本身就是整数，可以直接作为数组下标。

优点：

- 查找快
- 不需要 map

缺点：

- 预分配空间大一些

## `WebServer` 核心函数

### `init(...)`

- 作用：保存服务器配置
- 特点：这里只记录参数，不做真正的网络初始化

### `trig_mode()`

- 作用：根据配置设置监听 fd 和连接 fd 的 LT / ET 模式

### `log_write()`

- 作用：初始化日志系统
- 根据配置决定是同步日志还是异步日志

### `sql_pool()`

- 作用：
  - 初始化数据库连接池
  - 从数据库把已有用户读入内存

### `thread_pool()`

- 作用：创建线程池

### `eventListen()`

- 作用：完成网络初始化
- 包括：
  - 创建监听 socket
  - 设置 `SO_LINGER`
  - `bind()`
  - `listen()`
  - 创建 epoll
  - 把监听 fd 加入 epoll
  - 创建信号管道
  - 注册 `SIGALRM` 和 `SIGTERM`

### `eventLoop()`

- 作用：主事件循环
- 核心逻辑：
  - `epoll_wait()`
  - 处理新连接
  - 处理异常连接
  - 处理信号
  - 处理读写事件
  - 处理超时定时器

## 为什么要把信号写进管道

因为信号是异步事件，不适合在信号处理函数里做复杂逻辑。  
把信号写进管道后，主线程就能在 epoll 中统一处理，这样就把“异步信号”转成了“同步 I/O 事件”。

## 面试里可以怎么说

> `WebServer` 这一层主要承担服务器总控职责。它负责初始化监听 socket、创建 epoll、注册信号、建立线程池和数据库连接池，并在事件循环里统一处理新连接、读写事件、信号事件和超时事件。也就是说，它是整个服务器运行时的调度中心。

