---
title: pthread_cond_init
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/线程同步
---
# pthread_cond_init

> [!info] 功能
> 初始化条件变量。

## 函数原型

- int pthread_cond_init([[linux系统编程/概念词条/pthread_cond_t|pthread_cond_t]] \*cond, const pthread_condattr_t \*attr);

## 依赖头文件

- `#include <pthread.h>`

## 输入参数

- `cond`：要初始化的条件变量对象。
- `attr`：属性对象，传 `NULL` 表示默认属性。

## 输出参数

- `cond` 会被初始化为可用状态。

## 返回值

- 成功返回 `0`。
- 失败返回错误码。

## 知识点补充

- 条件变量通常要和互斥锁配套使用，单独使用没有意义。
- 它解决的是“线程什么时候该睡、什么时候该醒”的同步问题。

## 常见用法

- 初始化条件变量。

## 易错点

- 使用前必须先初始化。

## 相关概念

- [[linux系统编程/概念词条/pthread_cond_t|pthread_cond_t]]

## 相关课时

- [[linux系统编程/课时笔记/10 线程同步/03 条件变量与生产者消费者|03 条件变量与生产者消费者]]

## 相关模块

- [[linux系统编程/10 线程同步|10 线程同步]]
