---
title: gdb
tags:
  - linux
  - 系统编程
  - 指令查询
  - 命令卡片
---
# gdb

> [!info] 功能
> 调试可执行程序。

## 语法

- `gdb app`
- `gdb ./app`

## 输入参数

- `app`：带调试信息的可执行文件。

## 输出

- 进入 GDB 交互调试环境。

## 常见命令

- `run`：运行程序
- `break`：打断点
- `next`：单步执行，不进入函数
- `step`：单步执行，进入函数
- `continue`：继续运行到下一个断点
- `print`：打印变量
- `backtrace`：查看调用栈
- `frame`：切换栈帧
- `list`：查看源代码
- `info locals`：查看当前局部变量

## 易错点

- 没有 `-g` 时，源码行号和变量信息会非常有限。

## 相关课时

- [[linux系统编程/课时笔记/01 Linux基础与开发环境/05 GDB与Makefile]]

