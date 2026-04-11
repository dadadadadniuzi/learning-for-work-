---
title: SIGALRM
tags:
  - linux
  - 系统编程
  - 概念词条
  - 信号
---
# SIGALRM

## 它是什么
- `alarm` 或 `setitimer` 触发的超时信号。

## 怎么理解
- 它常被用来做超时控制。
- 默认动作通常是终止进程，但实际行为取决于程序是否捕捉它。

## 相关入口
- [[linux系统编程/函数笔记/信号/alarm|alarm]]
- [[linux系统编程/函数笔记/信号/setitimer|setitimer]]
- [[linux系统编程/概念词条/定时器|定时器]]
