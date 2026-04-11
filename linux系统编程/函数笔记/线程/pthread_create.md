---
title: pthread_create
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/线程
---
# pthread_create

> [!info] 功能
> 创建线程。

## 函数原型

- `int pthread_create([[linux系统编程/概念词条/pthread_t|pthread_t]] *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg);`

## 依赖头文件

- `#include <pthread.h>`

## 输入参数

- `thread`：输出线程 ID。
- `attr`：线程属性，传 `NULL` 表示默认属性。
- `start_routine`：线程入口函数，参数和返回值都按 `void *` 组织。
- `arg`：传给入口函数的参数指针。

## 输出参数

- `thread` 会接收到新线程的 ID。

## 返回值

- 成功返回 `0`。
- 失败返回错误码。

## 知识点补充

- 新线程会和创建者共享地址空间、[[linux系统编程/概念词条/文件描述符|文件描述符]]和很多进程资源。
- 线程入口函数的参数通常需要自己设计结构体来传递复杂数据。
- `[[linux系统编程/概念词条/pthread_t|pthread_t]]` 是线程标识，不要把它当普通整数随便比较或打印。

## 常见用法

- 在并发场景里启动一个工作线程。

## 易错点

- 入口函数签名必须严格匹配。
- `arg` 指针指向的数据生命周期要足够长。

## 相关概念

- [[linux系统编程/概念词条/pthread_t|pthread_t]]

## 相关课时

- [[linux系统编程/课时笔记/09 线程基础/02 pthread_create与pthread_self|02 pthread_create与pthread_self]]

## 相关模块

- [[linux系统编程/09 线程基础|09 线程基础]]
