---
title: INADDR_ANY
tags:
  - linux
  - 网络编程
  - 概念词条
---
# INADDR_ANY

## 它是什么

- `INADDR_ANY` 是 IPv4 编程中常用的特殊地址常量。
- 服务器绑定它时，表示可以接收发往本机任意网卡地址的连接。

## 常见写法

```c
servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
```

## 怎么理解

- 如果机器有多个网卡或多个 IP，绑定 `INADDR_ANY` 表示不限定某一个具体 IP。
- 客户端仍然需要连接服务器某个实际可达的 IP 地址。

## 易错点

- `INADDR_ANY` 常用于服务器端 `bind`，不是客户端连接目标地址。
- 它需要放进网络字节序字段，所以常配合 `htonl`。

## 常见出现位置

- [[linux网络编程/概念词条/sockaddr_in|sockaddr_in]]
- [[linux网络编程/函数笔记/Socket/bind|bind]]
