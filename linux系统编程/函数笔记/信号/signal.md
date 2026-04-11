---
title: signal
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/信号
---
# signal

> [!info] 功能
> 设置简单的[[linux系统编程/概念词条/信号处理方式|信号处理方式]]。

## 函数原型

- `void (*signal(int signum, void (*sighandler)(int)))(int);`
- `typedef void (*sighandler_t)(int);`
	`sighandler_t signal(int signum, sighandler_t handler);`

## 依赖头文件

- `#include <signal.h>`

## 输入参数

- `signum`：要处理的信号编号。
- `sighandler`：处理方式，可以是自定义捕捉函数、[[linux系统编程/概念词条/信号处理方式|SIG_IGN]] 忽略，或 [[linux系统编程/概念词条/信号默认动作|SIG_DFL]] 恢复默认动作。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回旧的[[linux系统编程/概念词条/信号处理方式|信号处理方式]]。
- 失败返回 `SIG_ERR`。

## 知识点补充

- `signal` 属于老式接口，语义相对简单，但在不同系统上的行为细节不如 [[linux系统编程/函数笔记/信号/sigaction.md|sigaction]] 稳定。
- 通过它可以把信号设为捕捉、忽略或恢复默认动作。
- 真正推荐的可控方式通常是 [[linux系统编程/函数笔记/信号/sigaction.md|sigaction]]。

## 常见用法

- 快速设置一个简单的信号处理逻辑。

## 易错点

- 不要把 `signal` 当作更强大的统一方案，复杂场景优先 [[linux系统编程/函数笔记/信号/sigaction.md|sigaction]]。

## 相关概念

- [[linux系统编程/概念词条/信号处理方式|信号处理方式]]
- [[linux系统编程/概念词条/信号默认动作|信号默认动作]]
- [[linux系统编程/概念词条/信号捕捉函数|信号捕捉函数]]

## 相关课时

- [[linux系统编程/课时笔记/06 信号/02 signal与sigaction|02 signal与sigaction]]

## 相关模块

- [[linux系统编程/07 信号|07 信号]]

