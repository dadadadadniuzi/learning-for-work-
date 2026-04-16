---
title: socket
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/Socket
---
# socket

> [!info] 功能
> 创建一个[[linux网络编程/概念词条/套接字|套接字]]，返回对应的文件描述符。

## 函数原型

- `int socket(int domain, int type, int protocol);`

## 依赖头文件

- `#include <sys/types.h>`
- `#include <sys/socket.h>`

## 输入参数

- `domain`：地址族，决定使用哪种通信域和地址结构。IPv4 使用 `AF_INET`，IPv6 使用 `AF_INET6`，本地套接字使用 `AF_UNIX` 或 `AF_LOCAL`。
- `type`：套接字类型，决定传输语义。`SOCK_STREAM` 通常表示 TCP 字节流，`SOCK_DGRAM` 通常表示 UDP 数据报。
- `protocol`：具体协议。多数情况下传 `0`，表示根据 `domain` 和 `type` 自动选择默认协议，例如 `AF_INET + SOCK_STREAM + 0` 通常选择 TCP。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回新的 socket 文件描述符。
- 失败返回 `-1`，并设置 `errno`。

## 知识点补充

- `socket` 只负责创建套接字，不负责绑定端口，也不负责建立连接。
- 服务器端创建后通常继续调用 [[linux网络编程/函数笔记/Socket/bind|bind]]、[[linux网络编程/函数笔记/Socket/listen|listen]]、[[linux网络编程/函数笔记/Socket/accept|accept]]。
- 客户端创建后通常调用 [[linux网络编程/函数笔记/Socket/connect|connect]]。

## 常见用法

```c
int lfd = socket(AF_INET, SOCK_STREAM, 0);
```

## 易错点

- `domain` 和后续地址结构必须匹配，`AF_INET` 对应 [[linux网络编程/概念词条/sockaddr_in|sockaddr_in]]。
- `SOCK_STREAM` 通常对应 [[linux网络编程/概念词条/TCP|TCP]]，不是“字符串流”的意思。

## 相关概念

- [[linux网络编程/概念词条/套接字|套接字]]
- [[linux网络编程/概念词条/地址族|地址族]]
- [[linux网络编程/概念词条/套接字类型|套接字类型]]
- [[linux网络编程/概念词条/TCP|TCP]]
- [[linux网络编程/概念词条/UDP|UDP]]
- [[linux网络编程/概念词条/AF_UNIX与AF_LOCAL|AF_UNIX与AF_LOCAL]]
- [[linux网络编程/概念词条/Unix Domain Socket|Unix Domain Socket]]

## 相关课时

- [[linux网络编程/课时笔记/02 Socket编程基础/01 套接字与socket模型|01 套接字与socket模型]]
- [[linux网络编程/课时笔记/02 Socket编程基础/05 socket-bind-listen-accept-connect|05 socket-bind-listen-accept-connect]]
- [[linux网络编程/课时笔记/06 本地套接字与通信总结/01 Unix Domain Socket基础|01 Unix Domain Socket基础]]

## 相关模块

- [[linux网络编程/02 Socket编程基础|02 Socket编程基础]]
- [[linux网络编程/06 本地套接字与通信总结|06 本地套接字与通信总结]]
