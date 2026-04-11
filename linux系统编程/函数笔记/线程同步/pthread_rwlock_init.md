---
title: pthread_rwlock_init
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/线程同步
---
# pthread_rwlock_init

> [!info] 功能
> 初始化读写锁。

## 函数原型

- int pthread_rwlock_init([[linux系统编程/概念词条/pthread_rwlock_t|pthread_rwlock_t]] \\*rwlock, const pthread_rwlockattr_t \\*attr);

## 依赖头文件

- `#include <pthread.h>`

## 输入参数

- `rwlock`：要初始化的读写锁对象。
- `attr`：属性对象，传 `NULL` 表示默认属性。

## 输出参数

- `rwlock` 会被初始化为可用状态。

## 返回值

- 成功返回 `0`。
- 失败返回错误码。

## 知识点补充

- 读写锁允许多个读者同时持有读锁，但写锁必须独占。
- 适合“读多写少”的共享数据场景。

## 常见用法

- 初始化读写锁对象。

## 易错点

- 使用前必须先初始化。

## 相关概念

- [[linux系统编程/概念词条/pthread_rwlock_t|pthread_rwlock_t]]

## 相关课时

- [[linux系统编程/课时笔记/10 线程同步/02 读写锁|02 读写锁]]

## 相关模块

- [[linux系统编程/10 线程同步|10 线程同步]]
