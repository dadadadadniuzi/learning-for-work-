---
title: bind
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/Socket
---
# bind

> [!info] 功能
> 给 socket 绑定本地地址，一般用于服务器绑定 IP 和端口。

## 函数原型

- `int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);`

## 依赖头文件

- `#include <sys/types.h>`
- `#include <sys/socket.h>`

## 输入参数

- `sockfd`：由 [[linux网络编程/函数笔记/Socket/socket|socket]] 返回的套接字文件描述符。服务器端通常绑定监听 socket。
- `addr`：指向本地地址结构的指针。IPv4 中实际准备的是 [[linux网络编程/概念词条/sockaddr_in|sockaddr_in]]，调用时强制转换为 `const struct sockaddr *`。
- `addrlen`：地址结构大小。IPv4 中常写 `sizeof(struct sockaddr_in)` 或 `sizeof(servaddr)`。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回 `0`。
- 失败返回 `-1`，并设置 `errno`。

## 知识点补充

- `bind` 绑定的是服务器本机地址，不是客户端地址。
- `sin_port` 要用 [[linux网络编程/函数笔记/网络字节序/htons|htons]] 转换。
- `sin_addr.s_addr` 可使用 `htonl(INADDR_ANY)` 表示监听本机所有网卡地址。

## 常见用法

```c
bind(lfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
```

## 易错点

- 调用 `bind` 前必须先正确初始化地址结构。
- 端口被占用时，`bind` 可能失败。
- 普通 TCP 服务器一般需要 `bind`，客户端通常不显式绑定端口。

## 相关概念

- [[linux网络编程/概念词条/sockaddr|sockaddr]]
- [[linux网络编程/概念词条/sockaddr_in|sockaddr_in]]
- [[linux网络编程/概念词条/socklen_t|socklen_t]]
- [[linux网络编程/概念词条/端口|端口]]
- [[linux网络编程/概念词条/INADDR_ANY|INADDR_ANY]]

## 相关课时

- [[linux网络编程/课时笔记/02 Socket编程基础/05 socket-bind-listen-accept-connect|05 socket-bind-listen-accept-connect]]
- [[linux网络编程/课时笔记/02 Socket编程基础/04 sockaddr与sockaddr_in结构|04 sockaddr与sockaddr_in结构]]

## 相关模块

- [[linux网络编程/02 Socket编程基础|02 Socket编程基础]]
