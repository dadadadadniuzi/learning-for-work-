---
title: sigemptyset
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/信号
---
# sigemptyset

> [!info] 功能
> 初始化一个空信号集。

## 函数原型

- int sigemptyset([[linux系统编程/概念词条/sigset_t|sigset_t]] \\*set);

## 依赖头文件

- `#include <signal.h>`

## 输入参数

- `set`：要初始化的信号集。

## 输出参数

- 会把 `set` 清空。

## 返回值

- 成功返回 `0`。
- 失败返回 `-1`，并设置 [[linux系统编程/概念词条/errno|errno]]。

## 知识点补充

- [[linux系统编程/概念词条/sigset_t|sigset_t]] 可以看成信号集合的抽象表示。
- `sigemptyset` 是在往信号集里加东西之前的第一步。

## 常见用法

- 构造信号屏蔽字或 [[linux系统编程/函数笔记/信号/sigaction.md|sigaction]] 的掩码。

## 易错点

- 不要忘记先初始化再使用信号集。

## 相关概念

- [[linux系统编程/概念词条/sigset_t|sigset_t]]

## 相关课时

- [[linux系统编程/课时笔记/06 信号/03 信号集与屏蔽|03 信号集与屏蔽]]

## 相关模块

- [[linux系统编程/07 信号|07 信号]]
