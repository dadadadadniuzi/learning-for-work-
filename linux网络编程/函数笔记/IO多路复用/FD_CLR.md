---
title: FD_CLR
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/IO多路复用
---
# FD_CLR

> [!info] 功能
> 将指定 fd 从 `fd_set` 集合中移除。

## 函数原型

- `void FD_CLR(int fd, fd_set *set);`

## 依赖头文件

- `#include <sys/select.h>`

## 输入参数

- `fd`：要从集合中移除的文件描述符。
- `set`：目标 [[linux网络编程/概念词条/fd_set|fd_set]] 集合指针。

## 输出参数

- `set`：调用后不再包含指定 `fd`。

## 返回值

- 无。

## 常见用法

```c
FD_CLR(cfd, &allset);
close(cfd);
```

## 易错点

- 客户端关闭后，要从集合中移除对应 fd，否则后续可能继续监听无效 fd。

## 相关概念

- [[linux网络编程/概念词条/fd_set|fd_set]]
- [[linux网络编程/概念词条/select模型|select模型]]

## 相关课时

- [[linux网络编程/课时笔记/05 IO多路复用/01 select|01 select]]

## 相关模块

- [[linux网络编程/05 IO多路复用|05 IO多路复用]]
