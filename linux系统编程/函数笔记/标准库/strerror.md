---
title: strerror
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/标准库
---
# strerror

> [!info] 功能
> 把错误码转换成可读的错误描述字符串。

## 函数原型

- char \*strerror(int errnum);

## 依赖头文件

- `#include <string.h>`

## 输入参数

- `errnum`：错误码，通常是 `errno`，也可以是像 `pthread_create`、`pthread_join` 这类线程函数直接返回的错误码。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功时返回一个指向错误描述字符串的指针。

## 知识点补充

- `strerror` 解决的是“我拿到了错误码，但还不知道它对应的人类可读含义”这个问题。
- 和 [[linux系统编程/概念词条/perror|perror]] 不同，`strerror` 不直接打印，而是返回字符串，方便你自己拼接到 `printf`、`fprintf`、日志输出里。
- 在线程库里，很多函数失败时是“直接返回错误码”，而不是设置 `errno`，这时常用 `strerror(ret)` 来解释这个返回值。

## 常见用法

- `printf("%s\n", strerror(errno));`
- `fprintf(stderr, "pthread_create error: %s\n", strerror(ret));`

## 易错点

- 不要把“线程函数直接返回的错误码”和 `errno` 混为一谈；线程函数里常常应该写 `strerror(ret)`，而不是 `strerror(errno)`。
- `strerror` 返回的是字符串指针，不是新分配的内存，一般不需要你自己 `free`。

## 相关概念

- [[linux系统编程/概念词条/errno|errno]]
- [[linux系统编程/概念词条/perror|perror]]

## 相关课时

- [[linux系统编程/课时笔记/09 线程基础/03 线程退出回收与分离|03 线程退出回收与分离]]

## 相关模块

- [[linux系统编程/09 线程基础|09 线程基础]]
