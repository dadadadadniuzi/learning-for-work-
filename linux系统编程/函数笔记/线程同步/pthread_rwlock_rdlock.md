---
title: pthread_rwlock_rdlock
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/线程同步
---
# pthread_rwlock_rdlock

> [!info] 功能
> 以读方式加锁。

## 函数原型

- `int pthread_rwlock_rdlock([[linux系统编程/概念词条/pthread_rwlock_t|pthread_rwlock_t]] *rwlock);`

## 依赖头文件

- `#include <pthread.h>`

## 输入参数

- `rwlock`：要操作的读写锁对象。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回 `0`。
- 失败返回错误码。

## 知识点补充

- `pthread_rwlock_rdlock` 表示共享读锁。
- 多个线程可以同时读，但写线程会被阻塞。

## 常见用法

- 保护以读为主的共享资源。

## 易错点

- 读锁不是完全无代价，仍然要和写锁规则协调。

## 相关概念

- [[linux系统编程/概念词条/pthread_rwlock_t|pthread_rwlock_t]]

## 相关课时

- [[linux系统编程/课时笔记/10 线程同步/02 读写锁|02 读写锁]]

## 相关模块

- [[linux系统编程/10 线程同步|10 线程同步]]
