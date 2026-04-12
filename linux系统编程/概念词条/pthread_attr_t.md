---
title: pthread_attr_t
tags:
  - linux
  - 系统编程
  - 概念词条
  - 线程
---
# pthread_attr_t

## 它是什么

- `pthread_attr_t` 是线程属性对象类型。
- 它用来描述“新线程应该以什么属性创建出来”，例如栈大小、分离状态、调度相关设置等。

## 常见理解

- 如果把 [[linux系统编程/函数笔记/线程/pthread_create.md|pthread_create]] 想成“创建线程”，那么 `pthread_attr_t` 就是“创建线程时的附加配置包”。
- 传 `NULL` 时，表示使用系统默认线程属性。

## 常见用途

- 指定线程是默认可连接态还是分离态。
- 调整线程栈大小。
- 在更复杂场景中配置调度继承或调度参数。

## 相关接口

- [[linux系统编程/函数笔记/线程/pthread_create.md|pthread_create]]
- [[linux系统编程/函数笔记/线程/pthread_attr_init.md|pthread_attr_init]]
- [[linux系统编程/函数笔记/线程/pthread_attr_setdetachstate.md|pthread_attr_setdetachstate]]
- [[linux系统编程/函数笔记/线程/pthread_attr_destroy.md|pthread_attr_destroy]]

## 学习建议

- 入门阶段先记住：大多数示例里 `attr` 传 `NULL` 就够了。
- 看到它时，要知道它不是线程 ID，也不是线程函数，而是“线程创建属性”。
- 真正要用它时，最常见流程是：初始化属性 -> 设置分离态等属性 -> 创建线程 -> 销毁属性对象。
