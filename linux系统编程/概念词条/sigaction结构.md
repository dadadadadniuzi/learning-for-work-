---
title: sigaction结构
tags:
  - linux
  - 系统编程
  - 概念词条
  - 信号
---
# sigaction结构

## 它是什么
- `struct sigaction` 是 `sigaction` 的配置结构体。
- 它定义了一个信号到来时应该怎么处理。

## 主要成员
- `sa_handler`：简单信号处理函数，或者 `SIG_IGN` / `SIG_DFL`。
- `sa_sigaction`：带更多上下文信息的处理函数，通常配合特定标志位使用。
- `sa_mask`：在处理当前信号期间额外屏蔽的信号集合。
- `sa_flags`：控制处理行为的标志位。
- `sa_restorer`：历史字段，课程里一般不重点使用。

## 怎么理解
- `sa_handler` / `sa_sigaction` 决定“谁来处理”。
- `sa_mask` 决定“处理时顺手屏蔽谁”。
- `sa_flags` 决定“处理规则怎么变”。

## 相关入口
- [[linux系统编程/函数笔记/信号/sigaction|sigaction]]
- [[linux系统编程/课时笔记/06 信号/02 signal与sigaction|02 signal与sigaction]]
