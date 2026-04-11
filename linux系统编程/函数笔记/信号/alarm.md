---
title: alarm
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/信号
---
# alarm

> [!info] 功能
> 设置一秒级的单次[[linux系统编程/概念词条/定时器|定时器]]。

## 函数原型

- `unsigned int alarm(unsigned int seconds);`

## 依赖头文件

- `#include <unistd.h>`

## 输入参数

- `seconds`：距离触发 [[linux系统编程/概念词条/SIGALRM|SIGALRM]] 的秒数。

## 输出参数

- 无直接输出参数。

## 返回值

- 返回上一个 `alarm` 剩余的秒数。

## 知识点补充

- `alarm` 会在指定秒数后产生 [[linux系统编程/概念词条/SIGALRM|SIGALRM]]。
- 它是最简单的定时方式，适合做超时控制。
- 如果要更精细的定时、周期定时或多种[[linux系统编程/概念词条/定时器|定时器]]，通常使用 [[linux系统编程/函数笔记/信号/setitimer|setitimer]]。

## 常见用法

- 为一个操作设置超时。

## 易错点

- 它只能以秒为单位，精度有限。

## 相关概念

- [[linux系统编程/概念词条/SIGALRM|SIGALRM]]
- [[linux系统编程/概念词条/定时器|定时器]]
- [[linux系统编程/函数笔记/信号/setitimer|setitimer]]

## 相关课时

- [[linux系统编程/课时笔记/06 信号/04 SIGCHLD与子进程回收|04 SIGCHLD与子进程回收]]

## 相关模块

- [[linux系统编程/07 信号|07 信号]]
