---
title: pthread_cond_timedwait
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/线程同步
---
# pthread_cond_timedwait

> [!info] 功能
> 在等待条件变量时增加“超时返回”能力。

## 函数原型

- int pthread_cond_timedwait([[linux系统编程/概念词条/pthread_cond_t|pthread_cond_t]] \*cond, [[linux系统编程/概念词条/pthread_mutex_t|pthread_mutex_t]] \*mutex, const struct [[linux系统编程/概念词条/timespec结构|timespec]] \*abstime);

## 依赖头文件

- `#include <pthread.h>`
- `#include <time.h>`

## 输入参数

- `cond`：要等待的条件变量。
- `mutex`：与条件变量配套使用的互斥锁。
- `abstime`：绝对超时时刻，类型是 `const struct` [[linux系统编程/概念词条/timespec结构|timespec]] `*`。到达这个时间点后如果条件还没满足，就结束等待并返回。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回 `0`，表示被正常唤醒并重新拿到锁。
- 超时返回 `ETIMEDOUT`。
- 其他失败情况返回错误码。

## 知识点补充

- `pthread_cond_timedwait` 可以理解成“带超时版本的 [[linux系统编程/函数笔记/线程同步/pthread_cond_wait|pthread_cond_wait]]”。
- 它内部同样会原子地“释放互斥锁并进入等待”，被唤醒或超时后再重新加锁。
- 这个接口特别适合“不想无限等待”的场景，比如等待任务、等待资源、等待某个状态变化。

## 常见用法

- 给条件等待加一个超时上限，避免线程永久阻塞。
- 在线程池、任务队列、超时控制里做定时等待。

## 易错点

- `abstime` 表示的是“绝对时间点”，不是“再等多少秒”。
- 即使被唤醒，也仍然要像 `pthread_cond_wait` 一样重新检查条件是否真的满足。

## 相关概念

- [[linux系统编程/概念词条/pthread_cond_t|pthread_cond_t]]
- [[linux系统编程/概念词条/pthread_mutex_t|pthread_mutex_t]]
- [[linux系统编程/概念词条/timespec结构|timespec结构]]
- [[linux系统编程/函数笔记/线程同步/pthread_cond_wait|pthread_cond_wait]]

## 相关课时

- [[linux系统编程/课时笔记/10 线程同步/03 条件变量与生产者消费者|03 条件变量与生产者消费者]]

## 相关模块

- [[linux系统编程/10 线程同步|10 线程同步]]
