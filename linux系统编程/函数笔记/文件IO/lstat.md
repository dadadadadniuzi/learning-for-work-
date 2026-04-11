---
title: lstat
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/文件IO
---
# lstat

> [!info] 功能
> 获取链接本身的文件状态。

## 函数原型

- int lstat(const char *pathname, struct [[linux系统编程/函数笔记/文件IO/stat.md|stat]] *statbuf);

## 依赖头文件

- #include <sys/types.h>
- #include <sys/stat.h>
- #include <unistd.h>

## 输入参数

- pathname：文件或链接路径。
- statbuf：输出结构体地址。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回 0。
- 失败返回 -1，并设置 `[[linux系统编程/概念词条/errno|errno]]`。

## 知识点补充

- `lstat` 不跟随软链接，而是读取链接本身的信息。
- 它特别适合判断一个路径到底是不是软链接。
- 硬链接和软链接的区别，经常要靠 `stat/lstat` 一起理解。

## 常见用法

- 分析链接类型和 [[linux系统编程/概念词条/inode|inode]] 关系。

## 易错点

- 别把 `stat` 和 `lstat` 混用到看不懂链接行为。

## 相关概念

- [[linux系统编程/概念词条/stat结构|stat结构]]
- [[linux系统编程/概念词条/inode|inode]]
- [[linux系统编程/概念词条/DIR|DIR]]
- [[linux系统编程/概念词条/dirent|dirent]]
- [[linux系统编程/概念词条/perror|perror]]

## 相关课时

- [[linux系统编程/课时笔记/02 文件IO/04 stat与lstat文件状态|04 stat与lstat文件状态]]

## 相关模块

- [[linux系统编程/02 文件IO|02 文件IO]]

