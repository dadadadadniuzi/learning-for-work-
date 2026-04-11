---
title: sigismember
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/信号
---
# sigismember

> [!info] 功能
> 判断某个信号是否属于指定的[[linux系统编程/概念词条/sigset_t|信号集]]。

## 函数原型

- int sigismember(const [[linux系统编程/概念词条/sigset_t|sigset_t]] \*set, int signum);

## 依赖头文件

- `#include <signal.h>`

## 输入参数

- `set`：要检查的信号集地址，类型是 `const` [[linux系统编程/概念词条/sigset_t|sigset_t]] `*`，这里只读，不会修改集合。
- `signum`：要判断的信号编号。

## 输出参数

- 无直接输出参数。

## 返回值

- 返回 `1`：说明 `signum` 在集合中。
- 返回 `0`：说明 `signum` 不在集合中。
- 返回 `-1`：说明出错，并设置 [[linux系统编程/概念词条/errno|errno]]。

## 知识点补充

- `sigismember` 更像“查询接口”，不负责修改集合。
- 它经常和 [[linux系统编程/函数笔记/信号/sigpending|sigpending]] 搭配，用来逐个判断哪些信号处于未决状态。
- 也可以配合 [[linux系统编程/函数笔记/信号/sigaddset|sigaddset]]、[[linux系统编程/函数笔记/信号/sigdelset|sigdelset]] 检查集合操作结果。

## 常见用法

- 遍历信号编号，检查某个集合中是否包含指定信号。
- 观察阻塞实验里哪些信号进入了未决集。

## 易错点

- 返回值不是传统的“成功 0 / 失败 -1”模式，`1` 和 `0` 本身就是有效查询结果。

## 相关概念

- [[linux系统编程/概念词条/sigset_t|sigset_t]]
- [[linux系统编程/函数笔记/信号/sigpending|sigpending]]
- [[linux系统编程/函数笔记/信号/sigaddset|sigaddset]]
- [[linux系统编程/函数笔记/信号/sigdelset|sigdelset]]
- [[linux系统编程/概念词条/errno|errno]]

## 相关课时

- [[linux系统编程/课时笔记/06 信号/03 信号集与屏蔽|03 信号集与屏蔽]]

## 相关模块

- [[linux系统编程/07 信号|07 信号]]
