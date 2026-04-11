---
title: pthread_self
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/线程
---
# pthread_self

> [!info] 功能
> 获取当前线程的 ID。

## 函数原型

- `[[linux系统编程/概念词条/pthread_t|pthread_t]] pthread_self(void);`

## 依赖头文件

- `#include <pthread.h>`

## 输入参数

- 无。

## 输出参数

- 无直接输出参数。

## 返回值

- 返回当前线程的 `[[linux系统编程/概念词条/pthread_t|pthread_t]]`。

## 知识点补充

- 它常用于在线程内部确认“我是谁”。
- 在调试输出里可以把线程 ID 打出来，但不要把它当作普通整数理解。

## 常见用法

- 线程内部打印自己的 ID。

## 易错点

- 线程 ID 的比较方式要遵循 `[[linux系统编程/概念词条/pthread_t|pthread_t]]` 类型语义。

## 相关概念

- [[linux系统编程/概念词条/pthread_t|pthread_t]]

## 相关课时

- [[linux系统编程/课时笔记/09 线程基础/02 pthread_create与pthread_self|02 pthread_create与pthread_self]]

## 相关模块

- [[linux系统编程/09 线程基础|09 线程基础]]
