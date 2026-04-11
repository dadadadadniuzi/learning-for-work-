---
title: pthread_mutex_init
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/线程同步
---
# pthread_mutex_init

> [!info] 功能
> 初始化互斥锁。

## 函数原型

- int pthread_mutex_init([[linux系统编程/概念词条/pthread_mutex_t|pthread_mutex_t]] \*mutex, const pthread_mutexattr_t \*attr);

## 依赖头文件

- `#include <pthread.h>`

## 输入参数

- `mutex`：要初始化的互斥锁对象。
- `attr`：互斥锁属性，传 `NULL` 表示默认属性。

## 输出参数

- `mutex` 会被初始化为可用状态。

## 返回值

- 成功返回 `0`。
- 失败返回错误码。

## 知识点补充

- 互斥锁用于保护临界区，保证同一时刻只有一个线程进入共享资源访问逻辑。
- 互斥锁初始化后才能使用，未初始化的对象不能直接加锁。

## 常见用法

- 初始化全局锁或局部锁。

## 易错点

- 使用前必须先初始化。

## 相关概念

- [[linux系统编程/概念词条/pthread_mutex_t|pthread_mutex_t]]

## 相关课时

- [[linux系统编程/课时笔记/10 线程同步/01 线程同步与互斥锁|01 线程同步与互斥锁]]

## 相关模块

- [[linux系统编程/10 线程同步|10 线程同步]]
