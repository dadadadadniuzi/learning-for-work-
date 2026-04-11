---
title: pthread_create与pthread_self
tags:
  - linux
  - 系统编程
  - 课时笔记
  - 系统编程/线程
---
# pthread_create与pthread_self

## 本节学什么

- `pthread_create`
- `pthread_self`
- 线程入口函数




## 本节学什么详解

- `pthread_create`：这一节围绕这个知识点展开，重点记住它解决什么问题、什么时候用、以及和前后内容的关系。
- `pthread_self`：这一节围绕这个知识点展开，重点记住它解决什么问题、什么时候用、以及和前后内容的关系。
- 线程入口函数：线程入口函数是新线程启动后真正执行的第一段代码，它的返回值通常会被线程退出机制接收。

## 知识点补充
- `pthread_create` 用来创建线程。
- `pthread_self` 用来获取当前线程 ID。
- 线程创建时要考虑入口函数签名和传参方式。

## 本节内容速览

- 创建线程时最关键的是入口函数和参数传递
- `pthread_create` 返回错误码而不是传统的 `-1`
- 打印线程 ID 能帮助理解并发执行



## 复习时要回答

- pthread_create到底解决什么问题？能不能用自己的话把它讲清楚？
- pthread_self和前后内容是什么关系，为什么这一节要把它单独拿出来讲？
- 如果你只允许保留一句话，这一节最该记住的结论是什么？

## 细节补充

- 线程共享地址空间、堆、全局变量，私有的是栈和线程上下文
- `pthread_create` 创建新线程后，入口函数会立即开始执行
- `pthread_join` 用于回收线程并获取线程返回值
- `pthread_detach` 表示线程结束后自动回收资源
- `pthread_cancel` 是取消请求，不是简单的强制杀死
## 本节关键函数

- [[linux系统编程/函数笔记/线程/pthread_create]]
- [[linux系统编程/函数笔记/线程/pthread_self]]


## 关联模块

- [[linux系统编程/09 线程基础]]



