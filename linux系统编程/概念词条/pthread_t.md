---
title: pthread_t
tags:
  - linux
  - 系统编程
  - 概念词条
  - 线程
---
# pthread_t

## 它是什么
- 线程标识类型。

## 怎么理解
- 它是线程的“身份证号”抽象，不一定就是普通整数。
- 用在 `pthread_create`、`pthread_self`、`pthread_join` 等接口里。

## 相关入口
- [[linux系统编程/函数笔记/线程/pthread_create|pthread_create]]
- [[linux系统编程/函数笔记/线程/pthread_self|pthread_self]]
- [[linux系统编程/函数笔记/线程/pthread_join|pthread_join]]
