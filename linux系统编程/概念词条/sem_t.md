---
title: sem_t
tags:
  - linux
  - 系统编程
  - 概念词条
  - 线程同步
---
# sem_t

## 它是什么

- `sem_t` 是 POSIX 信号量对象类型。
- 你可以把它理解成“信号量变量本身的类型”。

## 怎么理解

- 如果 [[linux系统编程/概念词条/信号量|信号量]] 是概念，那么 `sem_t` 就是这个概念在代码里的对象类型。
- 和 `pthread_mutex_t` 是互斥锁对象类型、`pthread_cond_t` 是条件变量对象类型类似，`sem_t` 对应的就是信号量对象。

## 常见出现位置

- [[linux系统编程/函数笔记/线程同步/sem_init|sem_init]]
- [[linux系统编程/函数笔记/线程同步/sem_wait|sem_wait]]
- [[linux系统编程/函数笔记/线程同步/sem_post|sem_post]]
- [[linux系统编程/函数笔记/线程同步/sem_destroy|sem_destroy]]

## 学习提示

- 看到 `sem_t` 时，要想到“这是一个信号量对象地址，后面通常会传给 `sem_*` 系列函数”。
