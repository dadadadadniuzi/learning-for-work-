---
title: htonl
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/网络字节序
---
# htonl

> [!info] 功能
> 将 32 位整数从主机字节序转换为[[linux网络编程/概念词条/网络字节序|网络字节序]]。

## 函数原型

- `uint32_t htonl(uint32_t hostlong);`

## 依赖头文件

- `#include <arpa/inet.h>`

## 输入参数

- `hostlong`：主机字节序下的 32 位无符号整数。网络编程中常用于 IPv4 地址整数，例如 `INADDR_ANY`。如果本机本来就是大端，可能不改变字节排列；如果本机是小端，会转换成大端。

## 输出参数

- 无直接输出参数。

## 返回值

- 返回转换后的 32 位网络字节序整数。

## 知识点补充

- `h` 表示 host，`n` 表示 network，`l` 表示 long。
- 课程中常见写法是 `htonl(INADDR_ANY)`。
- 和 [[linux网络编程/函数笔记/网络字节序/htons|htons]] 的区别是处理宽度不同，`htonl` 处理 32 位，`htons` 处理 16 位。

## 常见用法

```c
servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
```

## 易错点

- 端口号通常是 16 位，应使用 [[linux网络编程/函数笔记/网络字节序/htons|htons]]，不要用 `htonl`。
- 字符串 IP 地址应优先使用 [[linux网络编程/函数笔记/地址转换/inet_pton|inet_pton]] 转换。

## 相关概念

- [[linux网络编程/概念词条/网络字节序|网络字节序]]
- [[linux网络编程/概念词条/大端与小端|大端与小端]]
- [[linux网络编程/概念词条/INADDR_ANY|INADDR_ANY]]

## 相关课时

- [[linux网络编程/课时笔记/02 Socket编程基础/02 网络字节序|02 网络字节序]]

## 相关模块

- [[linux网络编程/02 Socket编程基础|02 Socket编程基础]]
