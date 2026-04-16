---
title: evutil_socket_t
tags:
  - linux
  - 网络编程
  - 概念词条
  - 网络编程/Libevent
---
# evutil_socket_t

`evutil_socket_t` 是 Libevent 对 socket 文件描述符类型的跨平台封装。

在 Linux 中，它本质上可以理解为一个整数文件描述符，和 [[linux系统编程/概念词条/文件描述符|文件描述符]] 的使用方式接近；但 Libevent 为了兼容 Windows，会使用 `evutil_socket_t` 这个类型名。

## 在课程里怎么理解

- 传给 [[linux网络编程/函数笔记/Libevent/event_new|event_new]] 时，它表示要监听的 fd。
- 传给 [[linux网络编程/函数笔记/Libevent/bufferevent_socket_new|bufferevent_socket_new]] 时，它表示要交给 bufferevent 管理的 socket。
- 在客户端主动连接时，可以先传 `-1`，再由 [[linux网络编程/函数笔记/Libevent/bufferevent_socket_connect|bufferevent_socket_connect]] 创建或绑定底层 socket。

## 易错点

- 在 Linux 学习阶段可以按 fd 理解它，但不要随意把它当普通指针使用。
- 如果 bufferevent 设置了 [[linux网络编程/概念词条/BEV_OPT_CLOSE_ON_FREE|BEV_OPT_CLOSE_ON_FREE]]，释放 bufferevent 时该 socket 会被自动关闭。

## 相关函数

- [[linux网络编程/函数笔记/Libevent/event_new|event_new]]
- [[linux网络编程/函数笔记/Libevent/bufferevent_socket_new|bufferevent_socket_new]]

## 相关模块

- [[linux网络编程/07 Libevent库|07 Libevent库]]
