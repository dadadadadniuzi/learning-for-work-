---
title: opendir
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/目录与文件系统
---
# opendir

> [!info] 功能
> 打开目录流，返回目录句柄。

## 函数原型

- `[[linux系统编程/概念词条/DIR|DIR]] *opendir(const char *name);`

## 依赖头文件

- `#include <[[linux系统编程/概念词条/dirent|dirent]].h>`

## 输入参数

- `name`：目录路径，可以是绝对路径，也可以是相对路径。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回 `[[linux系统编程/概念词条/DIR|DIR]] *` 目录流指针。
- 失败返回 `NULL`，并设置 `[[linux系统编程/概念词条/errno|errno]]`。

## 知识点补充

- `[[linux系统编程/概念词条/DIR|DIR]]` 不是普通[[linux系统编程/概念词条/文件描述符|文件描述符]]，而是目录流句柄。
- 打开目录后要配合 `[[linux系统编程/函数笔记/目录与文件系统/readdir.md|readdir]]` 顺序读取目录项，再用 `[[linux系统编程/函数笔记/目录与文件系统/closedir.md|closedir]]` 关闭。
- 目录遍历本质上是读目录项，不是像读普通文本一样按字节读取。

## 常见用法

- 开始遍历目录前先打开目录流。

## 易错点

- 不要把 `[[linux系统编程/概念词条/DIR|DIR]] *` 当成 `int fd`。

## 相关概念

- [[linux系统编程/概念词条/DIR|DIR]]
- [[linux系统编程/概念词条/dirent|dirent]]
- [[linux系统编程/概念词条/errno|errno]]
- [[linux系统编程/概念词条/perror|perror]]

## 相关课时

- [[linux系统编程/课时笔记/03 目录与文件系统/01 目录遍历基础|01 目录遍历基础]]

## 相关模块

- [[linux系统编程/03 目录与文件系统|03 目录与文件系统]]
