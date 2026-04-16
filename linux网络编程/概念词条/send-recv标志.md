---
title: send-recv标志
tags:
  - linux
  - 网络编程
  - 概念词条
---
# send-recv标志

## 它是什么

- `send` 和 `recv` 的 `flags` 参数用于控制本次发送或接收的特殊行为。
- 基础 TCP 通信中通常传 `0`，表示使用默认行为。

## 常见取值

- `0`：默认发送或接收方式，课程基础阶段最常用。
- `MSG_DONTWAIT`：本次调用使用非阻塞方式。
- `MSG_PEEK`：接收时查看数据但不从接收缓冲区移除。
- `MSG_NOSIGNAL`：发送时避免因对端关闭导致产生 `SIGPIPE`。

## 怎么理解

- `flags` 是对单次调用行为的补充控制。
- 初学阶段先把它传 `0`，把 TCP 连接和读写流程理解清楚。

## 易错点

- `flags = 0` 不是错误，而是最常见的默认用法。
- 不要在还没理解阻塞/非阻塞前随意使用 `MSG_DONTWAIT`。

## 常见出现位置

- [[linux网络编程/函数笔记/Socket/send|send]]
- [[linux网络编程/函数笔记/Socket/recv|recv]]
