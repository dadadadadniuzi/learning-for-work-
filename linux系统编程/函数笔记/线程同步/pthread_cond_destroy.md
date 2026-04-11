---
title: pthread_cond_destroy
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/线程同步
---
# pthread_cond_destroy

> [!info] 功能
> 销毁条件变量。

## 函数原型

- `int pthread_cond_destroy([[linux系统编程/概念词条/pthread_cond_t|pthread_cond_t]] *cond);`

## 依赖头文件

- `#include <pthread.h>`

## 输入参数

- `cond`：要销毁的条件变量对象。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回 `0`。
- 失败返回错误码。

## 知识点补充

- `pthread_cond_destroy` 用来销毁条件变量。
- 销毁前要确保没有线程在等待它。

## 常见用法

- 程序退出前清理条件变量。

## 易错点

- 销毁前要确认没有等待者。

## 相关概念

- [[linux系统编程/概念词条/pthread_cond_t|pthread_cond_t]]

## 相关课时

- [[linux系统编程/课时笔记/10 线程同步/03 条件变量与生产者消费者|03 条件变量与生产者消费者]]

## 相关模块

- [[linux系统编程/10 线程同步|10 线程同步]]
