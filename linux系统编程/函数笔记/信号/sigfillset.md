---
title: sigfillset
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/信号
---
# sigfillset

> [!info] 功能
> 把[[linux系统编程/概念词条/sigset_t|信号集]]填充为“几乎所有常规信号都在集合里”。

## 函数原型

- int sigfillset([[linux系统编程/概念词条/sigset_t|sigset_t]] \*set);

## 依赖头文件

- `#include <signal.h>`

## 输入参数

- `set`：要初始化的信号集地址，类型是 [[linux系统编程/概念词条/sigset_t|sigset_t]] `*`。函数会把这个集合填充成“满集”。

## 输出参数

- `set`：执行成功后，集合里会包含几乎所有可屏蔽的常规信号。

## 返回值

- 成功返回 `0`。
- 失败返回 `-1`，并设置 [[linux系统编程/概念词条/errno|errno]]。

## 知识点补充

- `sigfillset` 和 [[linux系统编程/函数笔记/信号/sigemptyset|sigemptyset]] 是相对的，一个构造空集，一个构造满集。
- 它不是说“所有信号都一定能被加入”，像 `SIGKILL`、`SIGSTOP` 这类不可屏蔽信号并不会真的被你屏蔽掉。
- 在实验里，它常用来快速构造“先全部加入，再删掉几个信号”的集合。

## 常见用法

- 快速构造一个大的信号集合。
- 配合 [[linux系统编程/函数笔记/信号/sigdelset|sigdelset]] 删除不想保留的信号。

## 易错点

- 不要误以为 `sigfillset` 之后就真的能屏蔽一切信号。
- 它只是修改集合内容，真正生效还要配合 [[linux系统编程/函数笔记/信号/sigprocmask|sigprocmask]]。

## 相关概念

- [[linux系统编程/概念词条/sigset_t|sigset_t]]
- [[linux系统编程/函数笔记/信号/sigemptyset|sigemptyset]]
- [[linux系统编程/函数笔记/信号/sigaddset|sigaddset]]
- [[linux系统编程/函数笔记/信号/sigdelset|sigdelset]]
- [[linux系统编程/概念词条/errno|errno]]

## 相关课时

- [[linux系统编程/课时笔记/06 信号/03 信号集与屏蔽|03 信号集与屏蔽]]

## 相关模块

- [[linux系统编程/07 信号|07 信号]]
