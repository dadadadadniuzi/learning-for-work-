---
title: connect
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/Socket
---
# connect

> [!info] 功能
> 客户端使用已有 socket 主动连接服务器地址。

## 函数原型

- `int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);`

## 依赖头文件

- `#include <sys/types.h>`
- `#include <sys/socket.h>`

## 输入参数

- `sockfd`：客户端通过 [[linux网络编程/函数笔记/Socket/socket|socket]] 创建的套接字文件描述符。
- `addr`：服务器地址结构指针。IPv4 中实际准备 [[linux网络编程/概念词条/sockaddr_in|sockaddr_in]]，填写服务器 IP 和端口后转换为 `const struct sockaddr *`。
- `addrlen`：服务器地址结构大小。IPv4 中常写 `sizeof(servaddr)`。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回 `0`。
- 失败返回 `-1`，并设置 `errno`。

## 知识点补充

- `connect` 是客户端连接服务器的关键函数。
- TCP 中 `connect` 成功意味着连接建立完成，后续可以使用该 socket 读写数据。
- 客户端通常不需要手动 `bind`，内核会自动选择本地 IP 和临时端口。

## 常见用法

```c
connect(cfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
```

## 易错点

- `addr` 里填写的是服务器地址，不是客户端地址。
- 服务器必须已经在对应 IP 和端口上监听，否则连接会失败。
- 字符串 IP 要用 [[linux网络编程/函数笔记/地址转换/inet_pton|inet_pton]] 填入地址结构。

## 相关概念

- [[linux网络编程/概念词条/已连接套接字|已连接套接字]]
- [[linux网络编程/概念词条/sockaddr|sockaddr]]
- [[linux网络编程/概念词条/sockaddr_in|sockaddr_in]]
- [[linux网络编程/概念词条/sockaddr_un|sockaddr_un]]
- [[linux网络编程/概念词条/TCP|TCP]]
- [[linux网络编程/概念词条/Unix Domain Socket|Unix Domain Socket]]

## 相关课时

- [[linux网络编程/课时笔记/02 Socket编程基础/05 socket-bind-listen-accept-connect|05 socket-bind-listen-accept-connect]]
- [[linux网络编程/课时笔记/06 本地套接字与通信总结/02 sockaddr_un与路径绑定|02 sockaddr_un与路径绑定]]

## 相关模块

- [[linux网络编程/02 Socket编程基础|02 Socket编程基础]]
- [[linux网络编程/06 本地套接字与通信总结|06 本地套接字与通信总结]]
