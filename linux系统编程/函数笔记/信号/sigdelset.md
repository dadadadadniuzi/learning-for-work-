---
title: sigdelset
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/信号
---
# sigdelset

> [!info] 功能
> 从[[linux系统编程/概念词条/sigset_t|信号集]]中删除一个指定信号。

## 函数原型

- int sigdelset([[linux系统编程/概念词条/sigset_t|sigset_t]] \*set, int signum);

## 依赖头文件

- `#include <signal.h>`

## 输入参数

- `set`：要操作的信号集地址，类型是 [[linux系统编程/概念词条/sigset_t|sigset_t]] `*`。
- `signum`：要从集合中移除的信号编号，例如 `SIGINT`、`SIGCHLD`。

## 输出参数

- `set`：执行成功后，这个集合里将不再包含 `signum`。

## 返回值

- 成功返回 `0`。
- 失败返回 `-1`，并设置 [[linux系统编程/概念词条/errno|errno]]。

## 知识点补充

- `sigdelset` 和 [[linux系统编程/函数笔记/信号/sigaddset|sigaddset]] 是相对操作。
- 它常见的套路是先用 [[linux系统编程/函数笔记/信号/sigfillset|sigfillset]] 构造大集合，再把不想保留的信号删掉。
- 它只改集合本身，不会自动修改进程当前的屏蔽字。

## 常见用法

- 从大集合中排除某几个特殊信号。
- 配合 [[linux系统编程/函数笔记/信号/sigismember|sigismember]] 检查删除是否成功。

## 易错点

- 删除集合元素不等于解除进程屏蔽；真正应用到进程上还要调用 [[linux系统编程/函数笔记/信号/sigprocmask|sigprocmask]]。

## 相关概念

- [[linux系统编程/概念词条/sigset_t|sigset_t]]
- [[linux系统编程/函数笔记/信号/sigfillset|sigfillset]]
- [[linux系统编程/函数笔记/信号/sigaddset|sigaddset]]
- [[linux系统编程/函数笔记/信号/sigismember|sigismember]]
- [[linux系统编程/概念词条/errno|errno]]

## 相关课时

- [[linux系统编程/课时笔记/06 信号/03 信号集与屏蔽|03 信号集与屏蔽]]

## 相关模块

- [[linux系统编程/07 信号|07 信号]]
