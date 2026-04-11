---
title: GDB调试流程
tags:
  - linux
  - 系统编程
  - 概念词条
  - 调试
---
# GDB调试流程

## 标准流程

1. 用 `gcc -g` 编译程序。
2. 用 `gdb app` 打开可执行文件。
3. 用 `break` 打断点。
4. 用 `run` 启动程序。
5. 用 `next` / `step` 逐步执行。
6. 用 `print`、`info locals`、`backtrace` 分析问题。
7. 定位后修改源码，再重新编译调试。

## 调试重点

- 看程序在哪一行停下。
- 看变量值是否符合预期。
- 看函数调用栈是否正确。

## 相关入口

- [[gdb]]
- [[linux系统编程/课时笔记/01 Linux基础与开发环境/05 GDB与Makefile]]

