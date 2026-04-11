---
title: pthread_rwlock_wrlock
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/线程同步
---
# pthread_rwlock_wrlock

> [!info] 功能
> 以写方式加锁。

## 函数原型

- `int pthread_rwlock_wrlock([[linux系统编程/概念词条/pthread_rwlock_t|pthread_rwlock_t]] *rwlock);`

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

- `pthread_rwlock_wrlock` 表示独占写锁。
- 一旦写锁占用，所有读线程和写线程都要等待。

## 常见用法

- 修改共享数据前加写锁。

## 易错点

- 写锁是独占的，别把它当成普通读锁。

## 相关概念

- [[linux系统编程/概念词条/pthread_rwlock_t|pthread_rwlock_t]]

## 相关课时

- [[linux系统编程/课时笔记/10 线程同步/02 读写锁|02 读写锁]]

## 相关模块

- [[linux系统编程/10 线程同步|10 线程同步]]
