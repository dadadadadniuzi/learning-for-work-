---
title: pthread_detach
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/线程
---
# pthread_detach

> [!info] 功能
> 把线程设置为分离状态。

## 函数原型

- `int pthread_detach(pthread_t thread);`

## 依赖头文件

- `#include <pthread.h>`

## 输入参数

- `thread`：要分离的线程 ID。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回 `0`。
- 失败返回错误码。

## 知识点补充

- 分离状态的线程结束后，系统会自动回收资源。
- 分离线程不能再被 `[[linux系统编程/函数笔记/线程/pthread_join.md|pthread_join]]` 回收。

## 常见用法

- 不需要回收返回值的后台线程。

## 易错点

- 一旦分离，就不要再对这个线程做 `join`。

## 相关概念

- [[linux系统编程/概念词条/pthread_t|pthread_t]]

## 相关课时

- [[linux系统编程/课时笔记/09 线程基础/03 线程退出回收与分离|03 线程退出回收与分离]]

## 相关模块

- [[linux系统编程/09 线程基础|09 线程基础]]
