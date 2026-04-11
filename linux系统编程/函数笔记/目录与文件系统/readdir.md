---
title: readdir
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/目录与文件系统
---
# readdir

> [!info] 功能
> 读取目录流中的下一个目录项。

## 函数原型

- struct [[linux系统编程/概念词条/dirent|dirent]] \*readdir([[linux系统编程/概念词条/DIR|DIR]] \*dirp);

## 依赖头文件

- `#include <dirent.h>`

## 输入参数

- `dirp`：已经通过 [[linux系统编程/函数笔记/目录与文件系统/opendir.md|opendir]] 打开的目录流。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回指向 struct [[linux系统编程/概念词条/dirent|dirent]] 的指针。
- 读到末尾或出错时返回 `NULL`，需要结合 [[linux系统编程/概念词条/errno|errno]] 判断。

## 知识点补充

- `readdir` 每次返回一个目录项，通常包含 `d_name`、`d_ino` 等字段。
- 目录项里会包含 `.` 和 `..`，遍历时一般要手动跳过。
- 返回的是目录流内部缓冲区指针，下一次调用前要及时使用。

## 常见用法

- 遍历目录下所有文件名。

## 易错点

- 不要长期保存 `readdir` 返回指针，后续调用可能覆盖。
- 末尾返回 `NULL` 不一定是错误，要结合 [[linux系统编程/概念词条/errno|errno]] 看。

## 相关概念

- [[linux系统编程/概念词条/DIR|DIR]]
- [[linux系统编程/概念词条/dirent|dirent]]
- [[linux系统编程/概念词条/inode|inode]]
- [[linux系统编程/概念词条/errno|errno]]

## 相关课时

- [[linux系统编程/课时笔记/03 目录与文件系统/01 目录遍历基础|01 目录遍历基础]]

## 相关模块

- [[linux系统编程/03 目录与文件系统|03 目录与文件系统]]
