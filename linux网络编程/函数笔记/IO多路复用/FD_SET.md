---
title: FD_SET
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/IO多路复用
---
# FD_SET

> [!info] 功能
> 将指定 fd 加入 `fd_set` 集合。

## 函数原型

- `void FD_SET(int fd, fd_set *set);`

## 依赖头文件

- `#include <sys/select.h>`

## 输入参数

- `fd`：要加入集合的文件描述符，例如监听 socket 或通信 socket。
- `set`：目标 [[linux网络编程/概念词条/fd_set|fd_set]] 集合指针。

## 输出参数

- `set`：调用后包含指定 `fd`。

## 返回值

- 无。

## 常见用法

```c
FD_SET(lfd, &allset);
```

## 易错点

- `fd` 不能超过 `fd_set` 支持范围。
- 加入新连接 fd 后，通常还要更新 `maxfd`。

## 相关概念

- [[linux网络编程/概念词条/fd_set|fd_set]]
- [[linux网络编程/概念词条/select模型|select模型]]

## 相关课时

- [[linux网络编程/课时笔记/05 IO多路复用/01 select|01 select]]

## 相关模块

- [[linux网络编程/05 IO多路复用|05 IO多路复用]]
