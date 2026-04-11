---
title: sighandler_t
tags:
  - linux
  - 系统编程
  - 概念词条
  - 信号
---
# sighandler_t

## 它是什么

- `sighandler_t` 是信号处理函数指针类型。
- 它表示“一个接收 `int` 类型信号编号、返回 `void` 的函数”。

## 对应原型

- `typedef void (*sighandler_t)(int);`

## 怎么理解

- 当你在 `signal` 里注册一个处理函数时，传进去的并不是普通变量，而是“函数地址”。
- 这个函数地址的类型，通常就可以用 `sighandler_t` 来表示。
- 所以它本质上是在描述“信号处理函数长什么样”。

## 常见出现位置

- [[linux系统编程/函数笔记/信号/signal.md|signal]]

## 可以传什么

- 你自己写的捕捉函数，例如 `void handler(int signo)`。
- [[linux系统编程/概念词条/信号处理方式|SIG_IGN]]，表示忽略信号。
- [[linux系统编程/概念词条/信号默认动作|SIG_DFL]]，表示恢复默认动作。

## 和 sighandler 的区别

- `sighandler_t` 是类型名。
- `sighandler` 往往只是函数原型里某个参数的名字。
- 所以看到 `void (*sighandler)(int)` 时，可以把它理解成“一个类型为 `sighandler_t` 的参数”。
