---
title: size_t
tags:
  - linux
  - 系统编程
  - 概念词条
  - 类型
---
# size_t

## 它是什么
- 用来表示大小、长度、字节数的无符号整数类型。

## 怎么理解
- 看到数组长度、缓冲区大小、对象字节数时，经常会遇到它。
- 它在 `malloc`、`read`、`write`、`getcwd`、`mmap` 等接口里很常见。

## 相关入口
- [[linux系统编程/函数笔记/文件IO/read|read]]
- [[linux系统编程/函数笔记/文件IO/write|write]]
- [[linux系统编程/函数笔记/文件IO/lseek|lseek]]
- [[linux系统编程/函数笔记/IPC/mmap|mmap]]
