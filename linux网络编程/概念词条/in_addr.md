---
title: in_addr
tags:
  - linux
  - 网络编程
  - 概念词条
---
# in_addr

## 它是什么

- `struct in_addr` 是 IPv4 地址的结构体类型。
- 它通常作为 [[linux网络编程/概念词条/sockaddr_in|sockaddr_in]] 的 `sin_addr` 字段出现。

## 常见原型

```c
struct in_addr {
    in_addr_t s_addr;
};
```

## 字段说明

- `s_addr`：保存 IPv4 地址的 32 位网络字节序整数。

## 常见写法

```c
servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr.s_addr);
```

## 易错点

- `s_addr` 不是字符串。
- `s_addr` 需要网络字节序。
- 如果使用 `inet_pton`，注意第三个参数通常传目标地址字段的地址。

## 常见出现位置

- [[linux网络编程/课时笔记/02 Socket编程基础/03 IP地址转换函数|03 IP地址转换函数]]
- [[linux网络编程/课时笔记/02 Socket编程基础/04 sockaddr与sockaddr_in结构|04 sockaddr与sockaddr_in结构]]
