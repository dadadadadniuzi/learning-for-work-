---
title: dup2
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/文件IO
---
# dup2

> [!info] 功能
> 按指定编号复制[[linux系统编程/概念词条/文件描述符|文件描述符]]。

## 函数原型

- int dup2(int oldfd, int newfd);

## 依赖头文件

- #include <unistd.h>

## 输入参数

- oldfd：源[[linux系统编程/概念词条/文件描述符|文件描述符]]。
- newfd：目标[[linux系统编程/概念词条/文件描述符|文件描述符]]编号。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回 `newfd`。
- 失败返回 -1，并设置 `[[linux系统编程/概念词条/errno|errno]]`。

## 知识点补充

- `dup2` 会先关闭 `newfd`，再把 `oldfd` 复制过去。
- 常用于标准输入、标准输出、标准错误重定向。
- 复制后两个 fd 仍共享同一个打开文件对象。

## 常见用法

- 把 [[linux系统编程/概念词条/标准文件描述符|STDIN_FILENO]]、[[linux系统编程/概念词条/标准文件描述符|STDOUT_FILENO]]、[[linux系统编程/概念词条/标准文件描述符|STDERR_FILENO]] 重定向到文件或设备。

## 易错点

- 别忘了 `newfd` 原来如果已打开，会先被关闭。

## 相关概念

- [[linux系统编程/概念词条/文件描述符|文件描述符]]
- [[linux系统编程/概念词条/标准文件描述符|标准文件描述符]]
- [[linux系统编程/概念词条/perror|perror]]

## 相关课时

- [[linux系统编程/课时笔记/02 文件IO/05 dup-dup2-fcntl与重定向|05 dup-dup2-fcntl与重定向]]

## 相关模块

- [[linux系统编程/02 文件IO|02 文件IO]]


