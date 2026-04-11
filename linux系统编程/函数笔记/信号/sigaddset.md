---
title: sigaddset
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/信号
---
# sigaddset

> [!info] 功能
> 向信号集中添加一个信号。

## 函数原型

- int sigaddset([[linux系统编程/概念词条/sigset_t|sigset_t]] \*set, int signum);

## 依赖头文件

- `#include <signal.h>`

## 输入参数

- `set`：要修改的信号集。
- `signum`：要加入的信号编号。

## 输出参数

- `set` 会被修改。

## 返回值

- 成功返回 `0`。
- 失败返回 `-1`，并设置 [[linux系统编程/概念词条/errno|errno]]。

## 知识点补充

- `sigaddset` 常用于构造屏蔽字。
- 它通常配合 [[linux系统编程/函数笔记/信号/sigemptyset.md|sigemptyset]] 和 [[linux系统编程/函数笔记/信号/sigprocmask.md|sigprocmask]] 使用。

## 常见用法

- 把某个信号加入屏蔽集。

## 易错点

- 使用前通常要先用 `sigemptyset` 初始化集合。

## 相关概念

- [[linux系统编程/概念词条/sigset_t|sigset_t]]
- [[linux系统编程/概念词条/信号阻塞|信号阻塞]]

## 相关课时

- [[linux系统编程/课时笔记/06 信号/03 信号集与屏蔽|03 信号集与屏蔽]]

## 相关模块

- [[linux系统编程/07 信号|07 信号]]
