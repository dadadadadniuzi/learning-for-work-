---
title: pthread_rwlock_destroy
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/线程同步
---
# pthread_rwlock_destroy

> [!info] 功能
> 销毁读写锁。

## 函数原型

- `int pthread_rwlock_destroy([[linux系统编程/概念词条/pthread_rwlock_t|pthread_rwlock_t]] *rwlock);`

## 依赖头文件

- `#include <pthread.h>`

## 输入参数

- `rwlock`：要销毁的读写锁对象。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回 `0`。
- 失败返回错误码。

## 知识点补充

- `pthread_rwlock_destroy` 用来销毁读写锁。
- 销毁前必须确保没有线程正在持有它。

## 常见用法

- 程序退出前清理锁资源。

## 易错点

- 销毁前先释放完所有读锁和写锁。

## 相关概念

- [[linux系统编程/概念词条/pthread_rwlock_t|pthread_rwlock_t]]

## 相关课时

- [[linux系统编程/课时笔记/10 线程同步/02 读写锁|02 读写锁]]

## 相关模块

- [[linux系统编程/10 线程同步|10 线程同步]]
