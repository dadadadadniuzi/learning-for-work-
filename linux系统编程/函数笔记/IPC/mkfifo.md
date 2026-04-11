---
title: mkfifo
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/IPC
---
# mkfifo

> [!info] 功能
> 创建有名管道 `FIFO`。

## 函数原型

- `int mkfifo(const char *pathname, mode_t mode);`

## 依赖头文件

- `#include <sys/types.h>`
- `#include <sys/stat.h>`

## 输入参数

- `pathname`：FIFO 文件路径。
- `mode`：创建时的权限位，会受到 [[linux系统编程/函数笔记/守护进程/umask.md|umask]] 影响。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回 `0`。
- 失败返回 `-1`，并设置 [[linux系统编程/概念词条/errno|errno]]。

## 知识点补充

- 有名管道会在文件系统里表现为一个特殊文件。
- 它可以用于无亲缘关系的进程通信。
- 打开 FIFO 时，如果另一端没有就绪，可能发生阻塞。

## 常见用法

- 作为跨进程的命名通信通道。

## 易错点

- 创建权限会被 `umask` 再屏蔽一层。

## 相关概念

- [[linux系统编程/概念词条/mode_t|mode_t]]
- [[linux系统编程/概念词条/umask|umask]]

## 相关课时

- [[linux系统编程/课时笔记/07 进程间通信 IPC/02 有名管道mkfifo|02 有名管道mkfifo]]

## 相关模块

- [[linux系统编程/06 进程间通信 IPC|06 进程间通信 IPC]]
