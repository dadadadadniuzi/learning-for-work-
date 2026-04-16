---
title: sockaddr
tags:
  - linux
  - 网络编程
  - 概念词条
---
# sockaddr

## 它是什么

- `struct sockaddr` 是 socket API 使用的通用地址结构类型。
- 很多函数为了兼容 IPv4、IPv6、本地套接字等不同地址格式，形参统一写成 `struct sockaddr *`。

## 常见原型

```c
struct sockaddr {
    sa_family_t sa_family;
    char        sa_data[14];
};
```

## 怎么理解

- `sockaddr` 更像一个通用外壳。
- 实际写 IPv4 程序时，我们通常准备的是 [[linux网络编程/概念词条/sockaddr_in|sockaddr_in]]。
- 调用 `bind/connect/accept` 时，再把 `struct sockaddr_in *` 强制转换成 `struct sockaddr *`。

## 常见写法

```c
struct sockaddr_in servaddr;
bind(fd, (struct sockaddr *)&servaddr, sizeof(servaddr));
```

## 易错点

- 不要直接手动填写 `sa_data` 来写 IPv4 地址，课程中通常使用 `sockaddr_in`。
- `(struct sockaddr *)&addr` 是为了匹配函数形参，不代表真实结构体内容变了。

## 常见出现位置

- [[linux网络编程/课时笔记/02 Socket编程基础/04 sockaddr与sockaddr_in结构|04 sockaddr与sockaddr_in结构]]
- [[linux网络编程/函数笔记/Socket/bind|bind]]
- [[linux网络编程/函数笔记/Socket/connect|connect]]
- [[linux网络编程/函数笔记/Socket/accept|accept]]
