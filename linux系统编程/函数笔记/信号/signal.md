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
- typedef void (`*`[[linux系统编程/概念词条/sighandler_t|sighandler_t]])(int);
- [[linux系统编程/概念词条/sighandler_t|sighandler_t]] signal(int signum, [[linux系统编程/概念词条/sighandler_t|sighandler_t]] handler);

## 依赖头文件

- `#include <signal.h>`

## 输入参数

- `signum`：要处理的信号编号。
- `sighandler/handler`：这是“信号处理函数类型”的形参名，本质上表示一个处理方式。它可以是你自己写的捕捉函数，也可以是 [[linux系统编程/概念词条/信号处理方式|SIG_IGN]] 表示忽略，或者 [[linux系统编程/概念词条/信号默认动作|SIG_DFL]] 表示恢复默认动作。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回旧的[[linux系统编程/概念词条/信号处理方式|信号处理方式]]。
- 失败返回 `SIG_ERR`。

## 知识点补充

- `signal` 属于老式接口，语义相对简单，但在不同系统上的行为细节不如 [[linux系统编程/函数笔记/信号/sigaction.md|sigaction]] 稳定。
- 原型里的 `sighandler` 只是“参数名”，真正更常见的说法是 [[linux系统编程/概念词条/sighandler_t|sighandler_t]]，也就是“接收一个 `int` 信号编号、返回 `void` 的函数指针类型”。
- 通过它可以把信号设为捕捉、忽略或恢复默认动作。
- 真正推荐的可控方式通常是 [[linux系统编程/函数笔记/信号/sigaction.md|sigaction]]。
- `void (*p)(int)`表示：
	p 是一个指针，指向“参数为 int、返回值为 void”的函数。
	

## 常见用法

- 快速设置一个简单的信号处理逻辑。

## 易错点

- 不要把 `signal` 当作更强大的统一方案，复杂场景优先 [[linux系统编程/函数笔记/信号/sigaction.md|sigaction]]。
- 尽量避免使用signal

## 相关概念

- [[linux系统编程/概念词条/sighandler_t|sighandler_t]]
- [[linux系统编程/概念词条/信号处理方式|信号处理方式]]
- [[linux系统编程/概念词条/信号默认动作|信号默认动作]]
- [[linux系统编程/概念词条/信号捕捉函数|信号捕捉函数]]

## 相关课时

- [[linux系统编程/课时笔记/06 信号/02 signal与sigaction|02 signal与sigaction]]

## 相关模块

- [[linux系统编程/07 信号|07 信号]]
