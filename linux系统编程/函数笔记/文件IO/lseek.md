---
title: lseek
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/文件IO
---
# lseek

> [!info] 功能
> 移动文件偏移量。

## 函数原型

- [[linux系统编程/概念词条/off_t|off_t]] lseek(int fd, [[linux系统编程/概念词条/off_t|off_t]] offset, int whence);

## 依赖头文件

- #include <unistd.h>

## 输入参数

- fd：支持随机访问的[[linux系统编程/概念词条/文件描述符|文件描述符]]。
- offset：偏移量。
- whence：基准位置，如 [[linux系统编程/概念词条/lseek基准|SEEK_SET]]、[[linux系统编程/概念词条/lseek基准|SEEK_CUR]]、[[linux系统编程/概念词条/lseek基准|SEEK_END]]。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回新的文件偏移量。
- 失败返回 -1，并设置 [[linux系统编程/概念词条/errno|errno]]。

## 知识点补充

- `lseek` 只改偏移量，不修改文件内容。
- 可用于获取文件长度，也可用于制造稀疏文件。
- 管道、FIFO 等并不适合随意 `lseek`。

## 常见用法

- 定位文件读写位置、获取文件大小、创建空洞。

## 易错点

- 别把 `lseek` 当成修改文件内容的接口。
- 只想改文件大小时通常直接用 [[linux系统编程/函数笔记/文件IO/ftruncate.md|ftruncate]] 更合适。

## 相关概念

- [[linux系统编程/概念词条/文件描述符|文件描述符]]
- [[linux系统编程/概念词条/lseek基准|lseek基准]]
- [[linux系统编程/概念词条/ftruncate|ftruncate]]
- [[linux系统编程/概念词条/errno|errno]]

## 相关课时

- [[linux系统编程/课时笔记/02 文件IO/03 lseek与文件偏移|03 lseek与文件偏移]]

## 相关模块

- [[linux系统编程/02 文件IO|02 文件IO]]


