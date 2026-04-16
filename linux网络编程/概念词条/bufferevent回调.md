---
title: bufferevent回调
tags:
  - linux
  - 网络编程
  - 概念词条
---
# bufferevent回调

## 它是什么

- bufferevent 回调用于处理 bufferevent 的读事件、写事件和连接状态事件。

## 常见回调

- 读回调：读缓冲区有数据时触发，常在里面调用 `bufferevent_read`。
- 写回调：写缓冲区数据被写出后触发，基础阶段可以先少用。
- 事件回调：连接成功、断开、错误等状态变化时触发。

## 常见设置函数

- [[linux网络编程/函数笔记/Libevent/bufferevent_setcb|bufferevent_setcb]]

## 易错点

- 设置回调不等于启用读写事件，还需要 `bufferevent_enable`。
- 读回调里读取的数据来自 bufferevent 的读缓冲区。

## 常见出现位置

- [[linux网络编程/课时笔记/07 Libevent库/05 bufferevent基础|05 bufferevent基础]]
