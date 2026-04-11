---
title: open
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/文件IO
---
# open

> [!info] 功能
> 打开或创建文件，返回[[linux系统编程/概念词条/文件描述符|文件描述符]]。

## 函数原型

- int open(const char *pathname, int flags, ...);

## 依赖头文件

- #include <[[linux系统编程/函数笔记/文件IO/fcntl.md|fcntl]].h>
- #include <sys/types.h>
- #include <sys/stat.h>

## 输入参数

- pathname：目标文件路径，支持相对路径和绝对路径。
- flags：打开方式与附加选项，如 [[linux系统编程/概念词条/open标志位|O_RDONLY]]、[[linux系统编程/概念词条/open标志位|O_WRONLY]]、[[linux系统编程/概念词条/open标志位|O_RDWR]]、[[linux系统编程/概念词条/open标志位|O_CREAT]]、[[linux系统编程/概念词条/open标志位|O_TRUNC]]、[[linux系统编程/概念词条/open标志位|O_APPEND]]。
- mode：仅在 [[linux系统编程/概念词条/open标志位|O_CREAT]] 时生效，表示新建文件权限，会受到 `[[linux系统编程/函数笔记/守护进程/umask.md|umask]]` 影响。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回新的[[linux系统编程/概念词条/文件描述符|文件描述符]]。
- 失败返回 -1，并设置 `[[linux系统编程/概念词条/errno|errno]]`。

## 知识点补充

- [[linux系统编程/概念词条/文件描述符|文件描述符]]是进程访问内核文件对象的入口。
- `flags` 决定打开方式，新建、截断、追加等行为都由它控制。
- `mode` 只在创建新文件时有意义，而且会被 `[[linux系统编程/函数笔记/守护进程/umask.md|umask]]` 屏蔽一部分权限位。

## 常见用法

- 打开文件、创建文件、做重定向前的基础步骤。

## 易错点

- 只要写了 [[linux系统编程/概念词条/open标志位|O_CREAT]]，就不要忘记传 `mode`。
- 失败时通常用 `[[linux系统编程/概念词条/perror|perror]]` 或查看 `[[linux系统编程/概念词条/errno|errno]]`。

## 相关概念

- [[linux系统编程/概念词条/文件描述符|文件描述符]]
- [[linux系统编程/概念词条/open标志位|open标志位]]
- [[linux系统编程/概念词条/errno|errno]]
- [[linux系统编程/概念词条/perror|perror]]
- [[linux系统编程/概念词条/umask|umask]]

## 相关课时

- [[linux系统编程/课时笔记/02 文件IO/01 文件描述符与open-close|01 文件描述符与open-close]]

## 相关模块

- [[linux系统编程/02 文件IO|02 文件IO]]


