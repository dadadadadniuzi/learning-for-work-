---
title: pthread_attr_destroy
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/线程
---
# pthread_attr_destroy

> [!info] 功能
> 销毁一个已经初始化过的[[linux系统编程/概念词条/pthread_attr_t|线程属性对象]]。

## 函数原型

- int pthread_attr_destroy([[linux系统编程/概念词条/pthread_attr_t|pthread_attr_t]] \*attr);

## 依赖头文件

- `#include <pthread.h>`

## 输入参数

- `attr`：要销毁的线程属性对象地址。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回 `0`。
- 失败返回错误码。

## 知识点补充

- `pthread_attr_destroy` 解决的是“属性对象用完之后怎样做收尾”这个问题。
- 它对应 [[linux系统编程/函数笔记/线程/pthread_attr_init.md|pthread_attr_init]]，前者初始化，后者销毁。
- 一般在线程已经创建完、属性对象不再需要时调用它。

## 常见用法

- `pthread_attr_init` -> `pthread_attr_setdetachstate` -> `pthread_create` -> `pthread_attr_destroy`

## 易错点

- 不要在属性对象还没初始化前就销毁它。

## 相关概念

- [[linux系统编程/概念词条/pthread_attr_t|pthread_attr_t]]
- [[linux系统编程/函数笔记/线程/pthread_attr_init.md|pthread_attr_init]]
- [[linux系统编程/函数笔记/线程/pthread_create.md|pthread_create]]

## 相关课时

- [[linux系统编程/课时笔记/09 线程基础/03 线程退出回收与分离|03 线程退出回收与分离]]

## 相关模块

- [[linux系统编程/09 线程基础|09 线程基础]]
