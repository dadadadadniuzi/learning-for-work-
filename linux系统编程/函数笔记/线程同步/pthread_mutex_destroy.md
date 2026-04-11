---
title: pthread_mutex_destroy
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/线程同步
---
# pthread_mutex_destroy

> [!info] 功能
> 销毁互斥锁。

## 函数原型

- int pthread_mutex_destroy([[linux系统编程/概念词条/pthread_mutex_t|pthread_mutex_t]] \\*mutex);

## 依赖头文件

- `#include <pthread.h>`

## 输入参数

- `mutex`：要销毁的互斥锁对象。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回 `0`。
- 失败返回错误码。

## 知识点补充

- `pthread_mutex_destroy` 用来销毁不再使用的互斥锁。
- 销毁前应确认没有线程正在持有该锁。

## 常见用法

- 程序退出前清理锁资源。

## 易错点

- 销毁前必须先保证锁已经释放完毕。

## 相关概念

- [[linux系统编程/概念词条/pthread_mutex_t|pthread_mutex_t]]

## 相关课时

- [[linux系统编程/课时笔记/10 线程同步/01 线程同步与互斥锁|01 线程同步与互斥锁]]

## 相关模块

- [[linux系统编程/10 线程同步|10 线程同步]]
