---
title: pthread_rwlock_unlock
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/线程同步
---
# pthread_rwlock_unlock

> [!info] 功能
> 释放读写锁。

## 函数原型

- int pthread_rwlock_unlock([[linux系统编程/概念词条/pthread_rwlock_t|pthread_rwlock_t]] \*rwlock);

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

- `pthread_rwlock_unlock` 释放当前持有的读写锁。
- 读锁和写锁都通过它释放。

## 常见用法

- 离开读区或写区时释放锁。

## 易错点

- 释放时要确认当前线程确实持有该锁。

## 相关概念

- [[linux系统编程/概念词条/pthread_rwlock_t|pthread_rwlock_t]]

## 相关课时

- [[linux系统编程/课时笔记/10 线程同步/02 读写锁|02 读写锁]]

## 相关模块

- [[linux系统编程/10 线程同步|10 线程同步]]
