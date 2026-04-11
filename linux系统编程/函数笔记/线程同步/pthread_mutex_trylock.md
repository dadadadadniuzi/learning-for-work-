---
title: pthread_mutex_trylock
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/线程同步
---
# pthread_mutex_trylock

> [!info] 功能
> 尝试加锁互斥锁。

## 函数原型

- int pthread_mutex_trylock([[linux系统编程/概念词条/pthread_mutex_t|pthread_mutex_t]] \*mutex);

## 依赖头文件

- `#include <pthread.h>`

## 输入参数

- `mutex`：要操作的互斥锁对象。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回 `0`。
- 失败返回错误码，锁被占用时通常会返回 `EBUSY`。

## 知识点补充

- `pthread_mutex_trylock` 不会阻塞等待，如果锁已被占用会立刻返回失败。
- 它适合对等待不敏感的场景。

## 常见用法

- 非阻塞地判断锁是否可用。

## 易错点

- 不要把它当成普通 `lock` 使用。

## 相关概念

- [[linux系统编程/概念词条/pthread_mutex_t|pthread_mutex_t]]

## 相关课时

- [[linux系统编程/课时笔记/10 线程同步/01 线程同步与互斥锁|01 线程同步与互斥锁]]

## 相关模块

- [[linux系统编程/10 线程同步|10 线程同步]]
