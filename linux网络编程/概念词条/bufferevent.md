---
title: bufferevent
tags:
  - linux
  - 网络编程
  - 概念词条
---
# bufferevent

## 它是什么

- `bufferevent` 是 Libevent 对网络读写事件和缓冲区的高级封装。
- 它把 socket、读缓冲区、写缓冲区、读写回调和事件回调组织到一个对象里。

## 怎么理解

- 普通 `event` 更像“fd 就绪了就通知我”。
- `bufferevent` 更像“帮我管理 socket 的读写缓冲和回调”。
- 网络通信中，`bufferevent` 比手动管理普通 `event` 更方便。

## 常见相关函数

- [[linux网络编程/函数笔记/Libevent/bufferevent_socket_new|bufferevent_socket_new]]
- [[linux网络编程/函数笔记/Libevent/bufferevent_setcb|bufferevent_setcb]]
- [[linux网络编程/函数笔记/Libevent/bufferevent_enable|bufferevent_enable]]
- [[linux网络编程/函数笔记/Libevent/bufferevent_read|bufferevent_read]]
- [[linux网络编程/函数笔记/Libevent/bufferevent_write|bufferevent_write]]

## 易错点

- 监听 fd 不应该直接作为每个客户端通信的 bufferevent fd。
- `bufferevent_setcb` 设置回调后，还要启用对应事件，例如 `EV_READ`。

## 常见出现位置

- [[linux网络编程/课时笔记/07 Libevent库/05 bufferevent基础|05 bufferevent基础]]
