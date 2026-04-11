---
title: sigset_t
tags:
  - linux
  - 系统编程
  - 概念词条
  - 信号
---
# sigset_t

## 它是什么
- `sigset_t` 是信号集合类型。
- 它可以表示一组信号，例如信号屏蔽字、未决信号集、`sigaction` 的额外屏蔽集。

## 它不是啥
- `sigset_t` 不是某一个具体信号。
- 它也不是简单的整数变量，而是“用来保存一组信号状态”的抽象类型。

## 常见用途
- 构造屏蔽字。
- 表示当前未决信号集合。
- 作为 `sigaction.sa_mask` 的类型。

## 相关操作
- [[linux系统编程/函数笔记/信号/sigemptyset|sigemptyset]]：把信号集清空。
- [[linux系统编程/函数笔记/信号/sigfillset|sigfillset]]：把几乎所有常规信号都加入集合。
- [[linux系统编程/函数笔记/信号/sigaddset|sigaddset]]：把一个指定信号加入集合。
- [[linux系统编程/函数笔记/信号/sigdelset|sigdelset]]：把一个指定信号从集合里删除。
- [[linux系统编程/函数笔记/信号/sigismember|sigismember]]：判断某个信号是否在集合中。

## 常见配套接口
- `sigemptyset`
- `sigfillset`
- `sigaddset`
- `sigdelset`
- `sigismember`
- `sigprocmask`
- `sigpending`

## 怎么理解
- 可以把它理解成“信号版的集合容器”。
- 需要先初始化，再添加或删除信号。
- 做实验时，经常会先用 [[linux系统编程/函数笔记/信号/sigemptyset|sigemptyset]] 或 [[linux系统编程/函数笔记/信号/sigfillset|sigfillset]] 初始化，再用 [[linux系统编程/函数笔记/信号/sigaddset|sigaddset]] / [[linux系统编程/函数笔记/信号/sigdelset|sigdelset]] 调整内容，最后用 [[linux系统编程/函数笔记/信号/sigismember|sigismember]] 判断结果。

## 相关入口
- [[linux系统编程/函数笔记/信号/sigemptyset|sigemptyset]]
- [[linux系统编程/函数笔记/信号/sigfillset|sigfillset]]
- [[linux系统编程/函数笔记/信号/sigaddset|sigaddset]]
- [[linux系统编程/函数笔记/信号/sigdelset|sigdelset]]
- [[linux系统编程/函数笔记/信号/sigismember|sigismember]]
- [[linux系统编程/函数笔记/信号/sigprocmask|sigprocmask]]
- [[linux系统编程/函数笔记/信号/sigpending|sigpending]]
