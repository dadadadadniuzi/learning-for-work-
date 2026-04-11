---
title: ftruncate
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/文件IO
---
# ftruncate

> [!info] 功能
> 调整已打开文件的长度。

## 函数原型

- int ftruncate(int fd, [[linux系统编程/概念词条/off_t|off_t]] length);

## 依赖头文件

- #include <unistd.h>

## 输入参数

- fd：已打开的[[linux系统编程/概念词条/文件描述符|文件描述符]]。
- length：目标长度。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回 0。
- 失败返回 -1，并设置 `[[linux系统编程/概念词条/errno|errno]]`。

## 知识点补充

- 它直接改文件大小，不是改偏移量。
- 长度变短会截断文件，变长则可能产生空洞。
- 常和 `mmap`、`[[linux系统编程/函数笔记/文件IO/lseek.md|lseek]]`、`open` 一起出现。

## 常见用法

- 创建或扩展文件映射前，先把文件长度准备好。

## 易错点

- 文件长度为 0 时做 `mmap` 往往会出错。

## 相关概念

- [[linux系统编程/概念词条/页对齐|页对齐]]
- [[linux系统编程/概念词条/mmap保护权限|mmap保护权限]]
- [[linux系统编程/概念词条/mmap映射标志|mmap映射标志]]
- [[linux系统编程/概念词条/MAP_FAILED|MAP_FAILED]]

## 相关课时

- [[linux系统编程/课时笔记/02 文件IO/03 lseek与文件偏移|03 lseek与文件偏移]]

## 相关模块

- [[linux系统编程/02 文件IO|02 文件IO]]

