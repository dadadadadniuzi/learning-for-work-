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

## 常见用途
- 构造屏蔽字。
- 表示当前未决信号集合。
- 作为 `sigaction.sa_mask` 的类型。

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

## 相关入口
- [[linux系统编程/函数笔记/信号/sigemptyset|sigemptyset]]
- [[linux系统编程/函数笔记/信号/sigaddset|sigaddset]]
- [[linux系统编程/函数笔记/信号/sigprocmask|sigprocmask]]
- [[linux系统编程/函数笔记/信号/sigpending|sigpending]]
