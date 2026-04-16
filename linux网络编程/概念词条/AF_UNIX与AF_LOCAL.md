---
title: AF_UNIX与AF_LOCAL
tags:
  - linux
  - 网络编程
  - 概念词条
---
# AF_UNIX与AF_LOCAL

## 它是什么

- `AF_UNIX` 和 `AF_LOCAL` 是 Unix Domain Socket 使用的本地通信地址族。
- 在 Linux 中它们通常表示同一种本地 socket 地址族。

## 怎么理解

- `AF_INET` 表示 IPv4 网络通信。
- `AF_UNIX` / `AF_LOCAL` 表示同机进程间通信。
- 选择这个地址族后，地址结构通常使用 [[linux网络编程/概念词条/sockaddr_un|sockaddr_un]]。

## 常见写法

```c
int fd = socket(AF_UNIX, SOCK_STREAM, 0);
addr.sun_family = AF_UNIX;
```

## 易错点

- `socket` 的地址族和地址结构里的 `sun_family` 要保持一致。
- 本地套接字不能用 `sockaddr_in`。

## 常见出现位置

- [[linux网络编程/课时笔记/06 本地套接字与通信总结/01 Unix Domain Socket基础|01 Unix Domain Socket基础]]
- [[linux网络编程/函数笔记/Socket/socket|socket]]
