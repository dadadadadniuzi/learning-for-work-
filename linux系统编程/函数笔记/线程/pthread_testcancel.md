---
title: pthread_testcancel
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/线程
---
# pthread_testcancel

> [!info] 功能
> 主动在当前线程里检查一次“是否有待处理的取消请求”。

## 函数原型

- void pthread_testcancel(void);

## 依赖头文件

- `#include <pthread.h>`

## 输入参数

- 无输入参数。它检查的是“当前线程自己”的取消状态，不需要你传线程 ID。

## 输出参数

- 无直接输出参数。

## 返回值

- 无返回值。
- 如果当前线程存在待处理取消请求，并且取消状态允许生效，那么线程会在这里作为取消点被终止。

## 知识点补充

- `pthread_testcancel` 本质上是“手动插入一个取消点”。
- [[linux系统编程/函数笔记/线程/pthread_cancel.md|pthread_cancel]] 发出的只是取消请求，真正何时结束线程，还要看目标线程什么时候到达取消点。
- 如果线程长期运行在没有取消点的循环里，可以主动调用 `pthread_testcancel` 让取消请求有机会被处理。

## 常见用法

- 在线程内部的长循环里周期性调用，保证线程能及时响应取消请求。
- 配合 [[linux系统编程/函数笔记/线程/pthread_cancel.md|pthread_cancel]] 做线程取消实验。

## 易错点

- 它不是“取消别人”的接口，而是“当前线程检查自己是否该取消”的接口。
- 如果线程已经把取消状态禁用，那么即使调用 `pthread_testcancel` 也不会终止线程。

## 相关概念

- [[linux系统编程/函数笔记/线程/pthread_cancel.md|pthread_cancel]]
- [[linux系统编程/概念词条/pthread_t|pthread_t]]

## 相关课时

- [[linux系统编程/课时笔记/09 线程基础/03 线程退出回收与分离|03 线程退出回收与分离]]

## 相关模块

- [[linux系统编程/09 线程基础|09 线程基础]]
