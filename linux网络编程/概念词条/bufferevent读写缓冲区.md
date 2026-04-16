---
title: bufferevent读写缓冲区
tags:
  - linux
  - 网络编程
  - 概念词条
---
# bufferevent读写缓冲区

## 它是什么

- bufferevent 内部维护读缓冲区和写缓冲区，用于缓存 socket 收到和待发送的数据。

## 怎么理解

- 数据到达 socket 后，Libevent 可将数据放入读缓冲区，然后触发读回调。
- 程序调用 `bufferevent_write` 时，数据先进入写缓冲区，再由 Libevent 负责写出。

## 常见函数

- [[linux网络编程/函数笔记/Libevent/bufferevent_read|bufferevent_read]]
- [[linux网络编程/函数笔记/Libevent/bufferevent_write|bufferevent_write]]

## 易错点

- `bufferevent_read` 是从 bufferevent 读缓冲区取数据，不是直接调用系统 `read`。
- `bufferevent_write` 成功不代表对端应用已经处理了数据。

## 常见出现位置

- [[linux网络编程/课时笔记/07 Libevent库/05 bufferevent基础|05 bufferevent基础]]
