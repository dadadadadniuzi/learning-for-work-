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
- `MSG_DONTWAIT`：本次调用使用 [[linux网络编程/概念词条/非阻塞I O|非阻塞I/O]] 方式。
- `MSG_PEEK`：接收时查看数据但不从接收缓冲区移除。
- `MSG_NOSIGNAL`：发送时避免因对端关闭导致产生 `SIGPIPE`。

## 怎么理解

- `flags` 是对单次调用行为的补充控制。
- 初学阶段先把它传 `0`，把 TCP 连接和读写流程理解清楚。
- 如果 fd 本身没有设置 `O_NONBLOCK`，也可以通过 `MSG_DONTWAIT` 只让这一次调用表现为非阻塞。

## 易错点

- `flags = 0` 不是错误，而是最常见的默认用法。
- 不要在还没理解 [[linux网络编程/概念词条/阻塞式IO|阻塞式IO]] / [[linux网络编程/概念词条/非阻塞I O|非阻塞I/O]] 前随意使用 `MSG_DONTWAIT`。
- `MSG_DONTWAIT` 是“本次调用非阻塞”，[[linux网络编程/概念词条/O_NONBLOCK|O_NONBLOCK]] 是把 fd 整体设为非阻塞，两者不要混淆。

## 常见出现位置

- [[linux网络编程/函数笔记/Socket/send|send]]
- [[linux网络编程/函数笔记/Socket/recv|recv]]
- [[linux网络编程/概念词条/非阻塞I O|非阻塞I/O]]
