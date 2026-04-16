---
title: FD_ZERO
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/IO多路复用
---
# FD_ZERO

> [!info] 功能
> 清空一个 `fd_set` 集合。

## 函数原型

- `void FD_ZERO(fd_set *set);`

## 依赖头文件

- `#include <sys/select.h>`

## 输入参数

- `set`：要清空的 [[linux网络编程/概念词条/fd_set|fd_set]] 集合指针，必须指向有效集合对象。

## 输出参数

- `set`：调用后集合中不包含任何 fd。

## 返回值

- 无。

## 常见用法

```c
fd_set allset;
FD_ZERO(&allset);
```

## 易错点

- 使用 `FD_SET` 前通常先 `FD_ZERO` 初始化集合。

## 相关概念

- [[linux网络编程/概念词条/fd_set|fd_set]]
- [[linux网络编程/概念词条/select模型|select模型]]

## 相关课时

- [[linux网络编程/课时笔记/05 IO多路复用/01 select|01 select]]

## 相关模块

- [[linux网络编程/05 IO多路复用|05 IO多路复用]]
