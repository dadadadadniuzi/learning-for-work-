---
title: pthread_cond_broadcast
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/线程同步
---
# pthread_cond_broadcast

> [!info] 功能
> 唤醒所有等待该条件变量的线程。

## 函数原型

- int pthread_cond_broadcast([[linux系统编程/概念词条/pthread_cond_t|pthread_cond_t]] \*cond);

## 依赖头文件

- `#include <pthread.h>`

## 输入参数

- `cond`：条件变量。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回 `0`。
- 失败返回错误码。

## 知识点补充

- `pthread_cond_broadcast` 会唤醒所有等待线程。
- 被唤醒的线程最终还要重新竞争互斥锁。

## 常见用法

- 状态变化后通知所有等待者重新检查条件。

## 易错点

- 广播不等于所有线程立刻继续执行，它们仍需重新抢锁。

## 相关概念

- [[linux系统编程/概念词条/pthread_cond_t|pthread_cond_t]]

## 相关课时

- [[linux系统编程/课时笔记/10 线程同步/03 条件变量与生产者消费者|03 条件变量与生产者消费者]]

## 相关模块

- [[linux系统编程/10 线程同步|10 线程同步]]
