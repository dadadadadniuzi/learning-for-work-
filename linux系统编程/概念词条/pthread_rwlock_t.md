---
title: pthread_rwlock_t
tags:
  - linux
  - 系统编程
  - 概念词条
  - 线程同步
---
# pthread_rwlock_t

## 它是什么
- 读写锁类型。

## 怎么理解
- 允许多个读者并发读，但写者独占。
- 适合读多写少场景。

## 相关入口
- [[linux系统编程/函数笔记/线程同步/pthread_rwlock_init|pthread_rwlock_init]]
- [[linux系统编程/函数笔记/线程同步/pthread_rwlock_rdlock|pthread_rwlock_rdlock]]
- [[linux系统编程/函数笔记/线程同步/pthread_rwlock_wrlock|pthread_rwlock_wrlock]]
