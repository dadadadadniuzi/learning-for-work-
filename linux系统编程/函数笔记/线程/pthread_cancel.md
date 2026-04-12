---
title: pthread_cancel
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/线程
---
# pthread_cancel

> [!info] 功能
> 向目标线程发送取消请求。

## 函数原型

- int pthread_cancel([[linux系统编程/概念词条/pthread_t|pthread_t]] thread);

## 依赖头文件

- `#include <pthread.h>`

## 输入参数

- `thread`：要取消的线程 ID。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回 `0`。
- 失败返回错误码，不是传统的 `-1` 加 `errno` 模式；实际开发里常配合 [[linux系统编程/函数笔记/标准库/strerror|strerror]] 解释这个返回值。

## 知识点补充

- `pthread_cancel` 发送的是“取消请求”，并不保证立刻停掉线程。
- 默认通常是延迟取消，线程会在取消点被真正终止。
- 如果，子线程没有到达取消点， 那么 pthread_cancel 无效。
- [[linux系统编程/函数笔记/线程/pthread_testcancel.md|pthread_testcancel]] 可以主动制造一个取消点，让线程及时响应取消请求。
- 是否允许取消、如何清理资源，还要配合线程自身的取消状态和清理处理。

## 常见用法

- 停止后台线程。

## 易错点

- 不要把取消和强制杀死等同。

## 相关概念

- [[linux系统编程/概念词条/pthread_t|pthread_t]]
- [[linux系统编程/函数笔记/线程/pthread_testcancel.md|pthread_testcancel]]
- [[linux系统编程/函数笔记/标准库/strerror|strerror]]

## 相关课时

- [[linux系统编程/课时笔记/09 线程基础/03 线程退出回收与分离|03 线程退出回收与分离]]

## 相关模块

- [[linux系统编程/09 线程基础|09 线程基础]]
