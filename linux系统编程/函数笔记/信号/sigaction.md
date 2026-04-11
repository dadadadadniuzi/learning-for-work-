---
title: sigaction
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/信号
---
# sigaction

> [!info] 功能
> 设置更可靠的信号处理方式。

## 函数原型

- int sigaction(int signum, const struct [[linux系统编程/概念词条/sigaction结构|sigaction]] \*act, struct [[linux系统编程/概念词条/sigaction结构|sigaction]] \*oldact);

## 依赖头文件

- `#include <signal.h>`

## 输入参数

- `signum`：目标信号。
- `act`：新的动作设置，里面通常会指定处理函数、屏蔽字和标志位。
- `oldact`：保存旧设置，可传 `NULL`。

## 输出参数

- `oldact` 会接收旧的 `sigaction` 配置。

## 返回值

- 成功返回 `0`。
- 失败返回 `-1`，并设置 [[linux系统编程/概念词条/errno|errno]]。

## 知识点补充

- `sigaction` 是 Linux 系统编程里设置信号处理的主力接口。
- struct [[linux系统编程/概念词条/sigaction结构|sigaction]] 里最重要的是 `sa_handler` / `sa_sigaction`、`sa_mask` 和 `sa_flags`。
- `sa_mask` 用来定义处理某个信号时额外屏蔽哪些信号。
- 和 [[linux系统编程/函数笔记/信号/signal.md|signal]] 相比，它更适合写出稳定的信号处理代码。

## 常见用法

- 注册自定义信号捕捉函数。

## 易错点

- 结构体里该填 `sa_handler` 还是 `sa_sigaction`，要和处理函数形式对应起来。

## 相关概念

- [[linux系统编程/概念词条/sigaction结构|sigaction结构]]
- [[linux系统编程/概念词条/sigset_t|sigset_t]]
- [[linux系统编程/概念词条/信号捕捉函数|信号捕捉函数]]
- [[linux系统编程/概念词条/信号默认动作|信号默认动作]]

## 相关课时

- [[linux系统编程/课时笔记/06 信号/02 signal与sigaction|02 signal与sigaction]]

## 相关模块

- [[linux系统编程/07 信号|07 信号]]
