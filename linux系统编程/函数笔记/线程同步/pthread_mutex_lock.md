---
title: pthread_mutex_lock
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/线程同步
---
# pthread_mutex_lock

> [!info] 功能
> 加锁互斥锁。

## 函数原型

- int pthread_mutex_lock([[linux系统编程/概念词条/pthread_mutex_t|pthread_mutex_t]] \\*mutex);

## 依赖头文件

- `#include <pthread.h>`

## 输入参数

- `mutex`：要操作的互斥锁对象。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回 `0`。
- 失败返回错误码。

## 知识点补充

- `pthread_mutex_lock` 会阻塞直到拿到锁。
- 它是最常见的临界区进入方式。

## 常见用法

- 进入共享数据的保护区。

## 易错点

- 加锁和解锁必须配对。

## 相关概念

- [[linux系统编程/概念词条/pthread_mutex_t|pthread_mutex_t]]

## 相关课时

- [[linux系统编程/课时笔记/10 线程同步/01 线程同步与互斥锁|01 线程同步与互斥锁]]

## 相关模块

- [[linux系统编程/10 线程同步|10 线程同步]]
