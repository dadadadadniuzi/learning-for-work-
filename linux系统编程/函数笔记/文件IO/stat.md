---
title: stat
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/文件IO
---
# stat

> [!info] 功能
> 获取文件状态信息。

## 函数原型

- int stat(const char *pathname, struct stat *statbuf);

## 依赖头文件

- `#include <sys/types.h>`
- `#include <sys/stat.h>`
- `#include <unistd.h>`
## 输入参数

- pathname：文件路径。
- statbuf：输出文件状态信息的结构体地址。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回 0。
- 失败返回 -1，并设置 [[linux系统编程/概念词条/errno|errno]]。

## 知识点补充

- `stat` 读取的是文件元信息，不是文件正文。
- 它会跟随软链接，读取目标文件的信息。
- 文件类型、权限、大小、时间戳、[[linux系统编程/概念词条/inode|inode]] 等都在这里。

## 常见用法

- 查看文件属性、判断文件类型、配合链接和目录操作。

## 易错点

- 如果想查看软链接本身而不是目标文件，要用 [[linux系统编程/函数笔记/文件IO/lstat.md|lstat]]。

## 相关概念

- [[linux系统编程/概念词条/stat结构|stat结构]]
- [[linux系统编程/概念词条/inode|inode]]
- [[linux系统编程/概念词条/mode_t|mode_t]]
- [[linux系统编程/概念词条/off_t|off_t]]
- [[linux系统编程/概念词条/size_t|size_t]]
- [[linux系统编程/概念词条/perror|perror]]

## 相关课时

- [[linux系统编程/课时笔记/02 文件IO/04 stat与lstat文件状态|04 stat与lstat文件状态]]

## 相关模块

- [[linux系统编程/02 文件IO|02 文件IO]]

