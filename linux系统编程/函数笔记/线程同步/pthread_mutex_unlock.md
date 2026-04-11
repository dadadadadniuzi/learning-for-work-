---
title: pthread_mutex_unlock
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/线程同步
---
# pthread_mutex_unlock

> [!info] 功能
> 解锁互斥锁。

## 函数原型

- int pthread_mutex_unlock([[linux系统编程/概念词条/pthread_mutex_t|pthread_mutex_t]] \\*mutex);

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

- `pthread_mutex_unlock` 释放互斥锁，让其他等待线程有机会进入临界区。
- 解锁和加锁必须按规则配对。

## 常见用法

- 离开临界区时释放锁。

## 易错点

- 不要忘记解锁，否则别的线程可能永远卡住。

## 相关概念

- [[linux系统编程/概念词条/pthread_mutex_t|pthread_mutex_t]]

## 相关课时

- [[linux系统编程/课时笔记/10 线程同步/01 线程同步与互斥锁|01 线程同步与互斥锁]]

## 相关模块

- [[linux系统编程/10 线程同步|10 线程同步]]
