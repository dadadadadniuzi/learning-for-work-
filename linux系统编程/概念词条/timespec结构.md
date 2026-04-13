---
title: timespec结构
tags:
  - linux
  - 系统编程
  - 概念词条
  - 时间
---
# timespec结构

## 它是什么

- `struct timespec` 是高精度时间结构体。
- 它通常用“秒 + 纳秒”的形式表示一个时间点或时间值。

## 主要成员

- `tv_sec`：秒。
- `tv_nsec`：纳秒。

## 怎么理解

- 如果说 `time_t` 更像“只有秒”的粗粒度时间表示，那么 `timespec` 就是在它基础上加了纳秒精度。
- 在 `pthread_cond_timedwait` 里，它通常表示“绝对超时时刻”，也就是线程最晚等到什么时候。

## 常见出现位置

- [[linux系统编程/函数笔记/线程同步/pthread_cond_timedwait|pthread_cond_timedwait]]

## 学习提示

- 这类接口里最容易混淆的是“相对等待多久”和“绝对等到什么时间点”。
- `pthread_cond_timedwait` 用的是“绝对时间点”，不是“相对时长”。
