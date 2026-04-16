---
title: FD_ISSET
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/IO多路复用
---
# FD_ISSET

> [!info] 功能
> 判断指定 fd 是否在 `fd_set` 集合中。

## 函数原型

- `int FD_ISSET(int fd, fd_set *set);`

## 依赖头文件

- `#include <sys/select.h>`

## 输入参数

- `fd`：要判断的文件描述符。
- `set`：目标 [[linux网络编程/概念词条/fd_set|fd_set]] 集合指针，通常是 `select` 返回后被修改过的集合。

## 输出参数

- 无直接输出参数。

## 返回值

- 非 0：`fd` 在集合中，通常表示该 fd 就绪。
- `0`：`fd` 不在集合中。

## 常见用法

```c
if (FD_ISSET(lfd, &rset)) {
    int cfd = accept(lfd, NULL, NULL);
}
```

## 易错点

- 应该在 `select` 返回后检查就绪集合，而不是检查原始全集合。

## 相关概念

- [[linux网络编程/概念词条/fd_set|fd_set]]
- [[linux网络编程/概念词条/事件就绪|事件就绪]]

## 相关课时

- [[linux网络编程/课时笔记/05 IO多路复用/01 select|01 select]]

## 相关模块

- [[linux网络编程/05 IO多路复用|05 IO多路复用]]
