---
title: pthread_exit
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/线程
---
# pthread_exit

> [!info] 功能
> 终止当前线程。

## 函数原型

- void pthread_exit(void \*retval);

## 依赖头文件

- `#include <pthread.h>`

## 输入参数

- `retval`：线程退出时返回给 [[linux系统编程/函数笔记/线程/pthread_join.md|pthread_join]] 的值。

## 输出参数

- 无直接输出参数。

## 返回值

- 无返回值，当前线程结束。

## 知识点补充

- 它只终止当前线程，不会让整个进程直接退出。
- `retval` 通常会在 [[linux系统编程/函数笔记/线程/pthread_join.md|pthread_join]] 中被取回。

## 常见用法

- 线程执行完毕后主动退出。

## 易错点

- 不要和进程级 `exit` 混淆。

## 相关概念

- [[linux系统编程/概念词条/pthread_t|pthread_t]]

## 相关课时

- [[linux系统编程/课时笔记/09 线程基础/03 线程退出回收与分离|03 线程退出回收与分离]]

## 相关模块

- [[linux系统编程/09 线程基础|09 线程基础]]
