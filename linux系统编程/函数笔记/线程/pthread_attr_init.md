---
title: pthread_attr_init
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/线程
---
# pthread_attr_init

> [!info] 功能
> 初始化一个[[linux系统编程/概念词条/pthread_attr_t|线程属性对象]]。

## 函数原型

- int pthread_attr_init([[linux系统编程/概念词条/pthread_attr_t|pthread_attr_t]] \*attr);

## 依赖头文件

- `#include <pthread.h>`

## 输入参数

- `attr`：要初始化的线程属性对象地址，类型是 [[linux系统编程/概念词条/pthread_attr_t|pthread_attr_t]] `*`。

## 输出参数

- `attr`：初始化成功后，会变成一个可用的默认线程属性对象。

## 返回值

- 成功返回 `0`。
- 失败返回错误码。

## 知识点补充

- `pthread_attr_init` 解决的是“怎样先得到一个可修改的线程属性对象”这个问题。
- 如果你想自己设置线程分离态、栈大小等属性，通常第一步就是先调用它。
- 如果根本不需要自定义线程属性，直接在 [[linux系统编程/函数笔记/线程/pthread_create.md|pthread_create]] 里传 `NULL` 即可。

## 常见用法

- 初始化属性对象后，再调用其他 `pthread_attr_*` 接口修改属性。

## 易错点

- 不要拿一个未初始化的 `pthread_attr_t` 直接去传给 [[linux系统编程/函数笔记/线程/pthread_create.md|pthread_create]]。

## 相关概念

- [[linux系统编程/概念词条/pthread_attr_t|pthread_attr_t]]
- [[linux系统编程/函数笔记/线程/pthread_attr_setdetachstate.md|pthread_attr_setdetachstate]]
- [[linux系统编程/函数笔记/线程/pthread_attr_destroy.md|pthread_attr_destroy]]

## 相关课时

- [[linux系统编程/课时笔记/09 线程基础/03 线程退出回收与分离|03 线程退出回收与分离]]

## 相关模块

- [[linux系统编程/09 线程基础|09 线程基础]]
