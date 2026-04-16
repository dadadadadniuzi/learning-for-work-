---
title: ntohs
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/网络字节序
---
# ntohs

> [!info] 功能
> 将 16 位整数从[[linux网络编程/概念词条/网络字节序|网络字节序]]转换为主机字节序。

## 函数原型

- `uint16_t ntohs(uint16_t netshort);`

## 依赖头文件

- `#include <arpa/inet.h>`

## 输入参数

- `netshort`：网络字节序下的 16 位无符号整数。网络编程中常来自 `sockaddr_in.sin_port`，表示对端或本端端口号。

## 输出参数

- 无直接输出参数。

## 返回值

- 返回转换后的主机字节序 16 位整数。

## 知识点补充

- `ntohs` 常用于打印客户端端口。
- 它和 [[linux网络编程/函数笔记/网络字节序/htons|htons]] 的方向相反。

## 常见用法

```c
printf("client port: %d\n", ntohs(cliaddr.sin_port));
```

## 易错点

- 从 `sin_port` 直接打印可能得到看起来很奇怪的数字，因为它还处于网络字节序。

## 相关概念

- [[linux网络编程/概念词条/网络字节序|网络字节序]]
- [[linux网络编程/概念词条/端口|端口]]

## 相关课时

- [[linux网络编程/课时笔记/02 Socket编程基础/02 网络字节序|02 网络字节序]]

## 相关模块

- [[linux网络编程/02 Socket编程基础|02 Socket编程基础]]
