---
title: pthread_cond_signal
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/线程同步
---
# pthread_cond_signal

> [!info] 功能
> 唤醒一个等待线程。

## 函数原型

- int pthread_cond_signal([[linux系统编程/概念词条/pthread_cond_t|pthread_cond_t]] \*cond);

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

- `pthread_cond_signal` 唤醒一个等待中的线程。
- 被唤醒的线程不一定立刻执行，因为它还要重新抢互斥锁。

## 常见用法

- 资源状态变化后通知一个等待者。

## 易错点

- 唤醒不等于条件已经满足，等待线程醒来后还是要检查条件。

## 相关概念

- [[linux系统编程/概念词条/pthread_cond_t|pthread_cond_t]]

## 相关课时

- [[linux系统编程/课时笔记/10 线程同步/03 条件变量与生产者消费者|03 条件变量与生产者消费者]]

## 相关模块

- [[linux系统编程/10 线程同步|10 线程同步]]
