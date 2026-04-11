---
title: setitimer类型
tags:
  - linux
  - 系统编程
  - 概念词条
  - 信号
---
# setitimer类型

## 它是什么
- `setitimer` 第一个参数 `which` 的取值。

## 常见取值
- `ITIMER_REAL`：真实时间，到点触发 [[linux系统编程/概念词条/SIGALRM|SIGALRM]]。
- `ITIMER_VIRTUAL`：只统计进程执行用户态 CPU 时间。
- `ITIMER_PROF`：统计用户态和内核态 CPU 时间。

## 怎么理解
- 它决定“按什么时间标准去计时”。
- [[linux系统编程/概念词条/setitimer类型|ITIMER_REAL]] 最常用于超时提醒和闹钟。

## 相关入口
- [[linux系统编程/函数笔记/信号/setitimer|setitimer]]
- [[linux系统编程/概念词条/SIGALRM|SIGALRM]]
