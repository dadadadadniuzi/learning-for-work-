---
title: sigprocmask操作
tags:
  - linux
  - 系统编程
  - 概念词条
  - 信号
---
# sigprocmask操作

## 它是什么
- `sigprocmask` 使用的操作方式参数。

## 常见取值
- `SIG_BLOCK`：把指定信号加入当前屏蔽字。
- `SIG_UNBLOCK`：把指定信号从当前屏蔽字移除。
- `SIG_SETMASK`：直接用新的集合覆盖当前屏蔽字。

## 怎么理解
- 它决定的是“怎么改屏蔽字”，而不是“具体屏蔽哪些信号”。
- 具体信号由 `sigset_t` 集合提供。

## 相关入口
- [[linux系统编程/函数笔记/信号/sigprocmask|sigprocmask]]
- [[linux系统编程/概念词条/sigset_t|sigset_t]]
