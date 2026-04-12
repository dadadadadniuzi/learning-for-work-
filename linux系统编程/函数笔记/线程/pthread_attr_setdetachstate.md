---
title: pthread_attr_setdetachstate
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/线程
---
# pthread_attr_setdetachstate

> [!info] 功能
> 设置[[linux系统编程/概念词条/pthread_attr_t|线程属性对象]]中的分离状态。

## 函数原型

- int pthread_attr_setdetachstate([[linux系统编程/概念词条/pthread_attr_t|pthread_attr_t]] \*attr, int detachstate);

## 依赖头文件

- `#include <pthread.h>`

## 输入参数

- `attr`：已经初始化好的线程属性对象地址。
- `detachstate`：要设置的分离状态，常见取值有 `PTHREAD_CREATE_JOINABLE` 和 `PTHREAD_CREATE_DETACHED`。

## 输出参数

- `attr`：设置成功后，属性对象里会记录新的分离状态。

## 返回值

- 成功返回 `0`。
- 失败返回错误码。

## 知识点补充

- `pthread_attr_setdetachstate` 解决的是“怎样在线程创建之前，就决定它是可连接态还是分离态”这个问题。
- 设成 `PTHREAD_CREATE_DETACHED` 后，用这个属性创建出来的线程退出时会自动回收资源，不再需要 [[linux系统编程/函数笔记/线程/pthread_join.md|pthread_join]]。
- 它和 [[linux系统编程/函数笔记/线程/pthread_detach.md|pthread_detach]] 的区别是：一个在创建前配置属性，一个在线程创建后再修改线程状态。

## 常见用法

- 想创建后台工作线程、日志线程这类“不需要回收返回值”的线程时，提前把属性设成分离态。

## 易错点

- 不要把 `PTHREAD_CREATE_DETACHED` 理解成“线程立刻运行在后台”；它真正决定的是“线程退出后资源是否需要别人回收”。

## 相关概念

- [[linux系统编程/概念词条/pthread_attr_t|pthread_attr_t]]
- [[linux系统编程/函数笔记/线程/pthread_detach.md|pthread_detach]]
- [[linux系统编程/函数笔记/线程/pthread_join.md|pthread_join]]
- [[linux系统编程/函数笔记/线程/pthread_create.md|pthread_create]]

## 相关课时

- [[linux系统编程/课时笔记/09 线程基础/03 线程退出回收与分离|03 线程退出回收与分离]]

## 相关模块

- [[linux系统编程/09 线程基础|09 线程基础]]
