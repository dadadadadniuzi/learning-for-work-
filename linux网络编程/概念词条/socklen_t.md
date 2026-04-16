---
title: socklen_t
tags:
  - linux
  - 网络编程
  - 概念词条
---
# socklen_t

## 它是什么

- `socklen_t` 是用于表示 socket 地址结构长度的整数类型。
- 它常出现在 `bind`、`connect`、`accept` 等函数的地址长度参数中。

## 怎么理解

- socket API 需要知道你传入的地址结构有多大。
- 对 IPv4 地址结构，常见写法是 `sizeof(struct sockaddr_in)` 或 `sizeof(addr)`。
- 在 `accept` 中，`socklen_t *addrlen` 是传入传出参数。

## 常见写法

```c
struct sockaddr_in cliaddr;
socklen_t len = sizeof(cliaddr);
int cfd = accept(lfd, (struct sockaddr *)&cliaddr, &len);
```

## 易错点

- `accept` 的第三个参数要传地址，即 `&len`，不能直接传 `len`。
- 调用 `accept` 前要先把 `len` 初始化为地址结构大小。

## 常见出现位置

- [[linux网络编程/函数笔记/Socket/bind|bind]]
- [[linux网络编程/函数笔记/Socket/connect|connect]]
- [[linux网络编程/函数笔记/Socket/accept|accept]]
