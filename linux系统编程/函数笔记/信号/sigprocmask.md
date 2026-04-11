---
title: sigprocmask
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/信号
---
# sigprocmask

> [!info] 功能
> 检查或修改进程的信号屏蔽字。

## 函数原型

- int sigprocmask(int how, const [[linux系统编程/概念词条/sigset_t|sigset_t]] \\*set, [[linux系统编程/概念词条/sigset_t|sigset_t]] \\*oldset);

## 依赖头文件

- `#include <signal.h>`

## 输入参数

- `how`：操作方式，常见有 [[linux系统编程/概念词条/sigprocmask操作|SIG_BLOCK]]、[[linux系统编程/概念词条/sigprocmask操作|SIG_UNBLOCK]]、[[linux系统编程/概念词条/sigprocmask操作|SIG_SETMASK]]。
- `set`：要操作的信号集。
- `oldset`：保存旧屏蔽字，可传 `NULL`。

## 输出参数

- `oldset` 会接收到旧的屏蔽字。

## 返回值

- 成功返回 `0`。
- 失败返回 `-1`，并设置 [[linux系统编程/概念词条/errno|errno]]。

## 知识点补充

- `sigprocmask` 改的是“信号屏蔽字”，它决定哪些信号暂时不能递达。
- 被屏蔽的信号不会丢失，而是会进入未决状态，等解除屏蔽后再递达。
- 这是解决信号竞态问题的重要工具。

## 常见用法

- 临时屏蔽某些信号。
- 在关键代码段前后保护状态。

## 易错点

- 阻塞信号不等于删除信号。
- 操作前先把信号集准备好。

## 相关概念

- [[linux系统编程/概念词条/sigset_t|sigset_t]]
- [[linux系统编程/概念词条/信号阻塞|信号阻塞]]
- [[linux系统编程/概念词条/信号未决|信号未决]]
- [[linux系统编程/概念词条/信号递达|信号递达]]

## 相关课时

- [[linux系统编程/课时笔记/06 信号/03 信号集与屏蔽|03 信号集与屏蔽]]

## 相关模块

- [[linux系统编程/07 信号|07 信号]]

