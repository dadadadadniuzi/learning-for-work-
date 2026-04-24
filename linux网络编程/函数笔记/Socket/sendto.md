---
title: sendto
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/Socket
---
# sendto

> [!info] 功能
> 向指定目标地址发送数据。UDP 编程中最常用，用来发送一个 UDP 数据报。

## 函数原型

- `ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);`

## 依赖头文件

- `#include <sys/types.h>`
- `#include <sys/socket.h>`

## 输入参数

- `sockfd`：socket 文件描述符。UDP 通常由 [[linux网络编程/函数笔记/Socket/socket|socket]] 创建，典型写法是 `socket(AF_INET, SOCK_DGRAM, 0)`。
- `buf`：要发送的数据缓冲区首地址。对于字符串可以传字符数组，对于二进制数据可以传任意内存地址。
- `len`：要发送的数据长度，类型是 [[linux网络编程/概念词条/size_t|size_t]]。注意它是字节数，不一定等于字符串长度加 `\0`。
- `flags`：发送标志。基础 UDP 通信中通常传 `0`。
- `dest_addr`：目标地址结构指针，类型通常由 `struct sockaddr_in *` 强制转换为 `struct sockaddr *`。UDP 客户端发送时这里填服务器地址；UDP 服务器回复时这里填客户端地址。
- `addrlen`：目标地址结构长度，类型是 [[linux网络编程/概念词条/socklen_t|socklen_t]]，IPv4 常传 `sizeof(struct sockaddr_in)`。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回实际发送的字节数。
- 失败返回 `-1`，并设置 `errno`。

## UDP 常见用法

```c
struct sockaddr_in servaddr;
servaddr.sin_family = AF_INET;
servaddr.sin_port = htons(9527);
inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);

sendto(fd, "hello", 5, 0,
       (struct sockaddr *)&servaddr, sizeof(servaddr));
```

## 在 UDP 通信中的作用

[[linux网络编程/函数笔记/Socket/sendto|sendto]] 每次发送时都可以指定目标地址，因此 UDP 客户端通常不需要 [[linux网络编程/函数笔记/Socket/connect|connect]]。

一次 [[linux网络编程/函数笔记/Socket/sendto|sendto]] 通常对应一个 UDP 数据报。接收端使用 [[linux网络编程/函数笔记/Socket/recvfrom|recvfrom]] 按数据报接收。

## 和 send 的区别

- [[linux网络编程/函数笔记/Socket/send|send]]：常用于已连接 socket，目标地址已经由连接关系确定。
- [[linux网络编程/函数笔记/Socket/sendto|sendto]]：可以在参数里指定目标地址，UDP 中更常见。

UDP socket 也可以先 [[linux网络编程/函数笔记/Socket/connect|connect]] 到默认目标，再用 [[linux网络编程/函数笔记/Socket/send|send]]，但入门案例通常直接使用 [[linux网络编程/函数笔记/Socket/sendto|sendto]]。

## 易错点

- `dest_addr` 要传地址结构指针，并强制转换为 `struct sockaddr *`。
- `addrlen` 要和目标地址结构匹配，IPv4 常用 `sizeof(struct sockaddr_in)`。
- UDP 不保证数据一定送达，[[linux网络编程/函数笔记/Socket/sendto|sendto]] 成功只表示数据交给了本机协议栈。
- 字符串发送时不要无脑发送整个数组大小，否则可能把未使用的缓冲区内容也发出去。

## 相关函数

- [[linux网络编程/函数笔记/Socket/recvfrom|recvfrom]]
- [[linux网络编程/函数笔记/Socket/socket|socket]]
- [[linux网络编程/函数笔记/Socket/bind|bind]]
- [[linux网络编程/函数笔记/Socket/send|send]]

## 相关概念

- [[linux网络编程/概念词条/UDP|UDP]]
- [[linux网络编程/概念词条/UDP通信流程|UDP通信流程]]
- [[linux网络编程/概念词条/UDP数据报格式|UDP数据报格式]]
- [[linux网络编程/概念词条/sockaddr_in|sockaddr_in]]
- [[linux网络编程/概念词条/socklen_t|socklen_t]]

## 相关课时

- [[linux网络编程/课时笔记/03 TCP通信与通信案例/03 UDP通信案例|03 UDP通信案例]]

## 相关模块

- [[linux网络编程/03 TCP通信与通信案例|03 TCP通信与通信案例]]
