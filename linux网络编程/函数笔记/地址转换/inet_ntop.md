---
title: inet_ntop
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/地址转换
---
# inet_ntop

> [!info] 功能
> 将网络字节序的二进制 IP 地址转换成人可读的字符串地址。

## 函数原型

- `const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);`

## 依赖头文件

- `#include <arpa/inet.h>`

## 输入参数

- `af`：地址族，决定按 IPv4 还是 IPv6 解释 `src`。IPv4 使用 `AF_INET`，IPv6 使用 `AF_INET6`。
- `src`：输入的二进制网络地址。IPv4 常传 `&addr.sin_addr` 或 `&addr.sin_addr.s_addr`。
- `dst`：输出字符串缓冲区，用于保存转换后的人可读 IP 字符串。
- `size`：`dst` 缓冲区大小。IPv4 常用 `INET_ADDRSTRLEN`，IPv6 常用 `INET6_ADDRSTRLEN`，避免缓冲区太小。

## 输出参数

- `dst`：成功时写入转换后的 IP 字符串，例如 `"127.0.0.1"`。

## 返回值

- 成功返回 `dst`。
- 失败返回 `NULL`，并设置 `errno`。

## 知识点补充

- `inet_ntop` 常用于服务器打印客户端 IP。
- 它和 [[linux网络编程/函数笔记/地址转换/inet_pton|inet_pton]] 是一对方向相反的转换函数。

## 常见用法

```c
char ip[INET_ADDRSTRLEN];
inet_ntop(AF_INET, &cliaddr.sin_addr, ip, sizeof(ip));
```

## 易错点

- `dst` 必须是可写缓冲区，不能传字符串常量。
- `size` 要传缓冲区大小，不是 IP 地址长度。

## 相关概念

- [[linux网络编程/概念词条/IP地址|IP地址]]
- [[linux网络编程/概念词条/sockaddr_in|sockaddr_in]]
- [[linux网络编程/概念词条/socklen_t|socklen_t]]

## 相关课时

- [[linux网络编程/课时笔记/02 Socket编程基础/03 IP地址转换函数|03 IP地址转换函数]]

## 相关模块

- [[linux网络编程/02 Socket编程基础|02 Socket编程基础]]
