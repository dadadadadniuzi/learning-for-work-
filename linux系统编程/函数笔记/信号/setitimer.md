---
title: setitimer
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/信号
---
# setitimer

> [!info] 功能
> 设置间隔[[linux系统编程/概念词条/定时器|定时器]]。

## 函数原型

- `int setitimer(int which, const struct itimerval *new_value, struct itimerval *old_value);`

## 依赖头文件

- `#include <sys/time.h>`
- `#include <signal.h>`

## 输入参数

- `which`：[[linux系统编程/概念词条/定时器|定时器]]类型，常见有 [[linux系统编程/概念词条/setitimer类型|ITIMER_REAL]]、[[linux系统编程/概念词条/setitimer类型|ITIMER_VIRTUAL]]、[[linux系统编程/概念词条/setitimer类型|ITIMER_PROF]]。
- `new_value`：新的[[linux系统编程/概念词条/定时器|定时器]]设置，里面包含首次触发时间和周期时间。
- `old_value`：保存旧[[linux系统编程/概念词条/定时器|定时器]]设置，可传 `NULL`。

## 输出参数

- `old_value` 会接收旧的[[linux系统编程/概念词条/定时器|定时器]]设置。

## 返回值

- 成功返回 `0`。
- 失败返回 `-1`，并设置 [[linux系统编程/概念词条/errno|errno]]。

## 知识点补充

- `itimerval` 结构体里有 `it_value` 和 `it_interval` 两部分，分别表示下一次触发和后续周期。
- [[linux系统编程/概念词条/setitimer类型|ITIMER_REAL]] 触发 [[linux系统编程/概念词条/SIGALRM|SIGALRM]]，[[linux系统编程/概念词条/setitimer类型|ITIMER_VIRTUAL]] 和 [[linux系统编程/概念词条/setitimer类型|ITIMER_PROF]] 则和进程运行时间相关。
- 它比 [[linux系统编程/函数笔记/信号/alarm.md|alarm]] 更灵活，能做周期性定时。

## 常见用法

- 实现周期定时。
- 替代简单 [[linux系统编程/函数笔记/信号/alarm.md|alarm]] 完成更复杂的超时控制。

## 易错点

- 不同 `which` 的计时语义不同，要分清楚。

## 相关概念

- [[linux系统编程/概念词条/itimerval结构|itimerval结构]]
- [[linux系统编程/概念词条/setitimer类型|setitimer类型]]
- [[linux系统编程/概念词条/SIGALRM|SIGALRM]]

## 相关课时

- [[linux系统编程/课时笔记/06 信号/04 SIGCHLD与子进程回收|04 SIGCHLD与子进程回收]]

## 相关模块

- [[linux系统编程/07 信号|07 信号]]
