---
title: pthread_cond_wait
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/线程同步
---
# pthread_cond_wait

> [!info] 功能
> 等待条件变量。

## 函数原型

- int pthread_cond_wait([[linux系统编程/概念词条/pthread_cond_t|pthread_cond_t]] \*cond, [[linux系统编程/概念词条/pthread_mutex_t|pthread_mutex_t]] \*mutex);

## 依赖头文件

- `#include <pthread.h>`

## 输入参数

- `cond`：条件变量。
- `mutex`：与条件变量配套的互斥锁。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回 `0`。
- 失败返回错误码。

## 知识点补充

- `pthread_cond_wait` 会在内部原子地“解锁 mutex 并进入等待”，被唤醒后再重新加锁。
- 因此它必须和互斥锁一起配合使用。
- 等待条件变量时通常要用 `while` 反复检查条件，避免虚假唤醒或条件被其他线程先消费。

## 常见用法

- 生产者消费者模型里消费者等待资源到来。

## 易错点

- `wait` 前必须先拿着对应互斥锁。
- 唤醒后不要假设条件一定已经满足，要重新检查。

## 相关概念

- [[linux系统编程/概念词条/pthread_cond_t|pthread_cond_t]]
- [[linux系统编程/概念词条/pthread_mutex_t|pthread_mutex_t]]

## 相关课时

- [[linux系统编程/课时笔记/10 线程同步/03 条件变量与生产者消费者|03 条件变量与生产者消费者]]

## 相关模块

- [[linux系统编程/10 线程同步|10 线程同步]]
