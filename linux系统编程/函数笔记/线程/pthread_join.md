---
title: pthread_join
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/线程
---
# pthread_join

> [!info] 功能
> 等待指定线程结束并回收资源。

## 函数原型

- int pthread_join([[linux系统编程/概念词条/pthread_t|pthread_t]] thread, void \\*\\*retval);

## 依赖头文件

- `#include <pthread.h>`

## 输入参数

- `thread`：要等待的线程 ID。
- `retval`：接收线程退出值，可传 `NULL`。

## 输出参数

- `retval` 会接收到线程退出值。

## 返回值

- 成功返回 `0`。
- 失败返回错误码。

## 知识点补充

- `pthread_join` 会阻塞当前线程，直到目标线程结束。
- 一个线程通常只能被 `join` 一次。
- 如果线程已经被分离，就不能再 `join`。

## 常见用法

- 线程收尾，回收返回值。

## 易错点

- 不要把可 join 和 detached 两种状态混在一起。

## 相关概念

- [[linux系统编程/概念词条/pthread_t|pthread_t]]

## 相关课时

- [[linux系统编程/课时笔记/09 线程基础/03 线程退出回收与分离|03 线程退出回收与分离]]

## 相关模块

- [[linux系统编程/09 线程基础|09 线程基础]]
