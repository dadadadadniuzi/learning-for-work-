---
title: pthread_cond_t
tags:
  - linux
  - 系统编程
  - 概念词条
  - 线程同步
---
# pthread_cond_t

## 它是什么
- 条件变量类型。

## 怎么理解
- 它本身不负责互斥，只负责“通知条件变化”。
- 通常要和互斥锁一起用。

## 相关入口
- [[linux系统编程/函数笔记/线程同步/pthread_cond_init|pthread_cond_init]]
- [[linux系统编程/函数笔记/线程同步/pthread_cond_wait|pthread_cond_wait]]
- [[linux系统编程/函数笔记/线程同步/pthread_cond_signal|pthread_cond_signal]]
