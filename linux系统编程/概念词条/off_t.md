---
title: off_t
tags:
  - linux
  - 系统编程
  - 概念词条
  - 文件IO
---
# off_t

## 它是什么
- 用来表示文件偏移量、文件长度等位置相关值的类型。

## 怎么理解
- 它最常见于 `lseek`、`mmap`、`stat` 这类和文件位置有关的接口。
- 看到文件“偏移”、“位置”、“长度”时，很容易遇到它。

## 相关入口
- [[linux系统编程/函数笔记/文件IO/lseek|lseek]]
- [[linux系统编程/函数笔记/IPC/mmap|mmap]]
- [[linux系统编程/概念词条/lseek基准|lseek基准]]
