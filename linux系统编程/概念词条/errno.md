---
title: errno
tags:
  - linux
  - 系统编程
  - 概念词条
  - 错误处理
---
# errno

## 它是什么
- 记录最近一次系统调用或库函数失败原因的错误码。

## 怎么理解
- 先看函数返回值，再看 `errno`。
- 成功时通常不该依赖 `errno` 的值。
- 它经常和 `perror` 一起用来做错误提示。

## 相关入口
- [[linux系统编程/概念词条/perror|perror]]
- [[linux系统编程/函数笔记/文件IO/open|open]]
- [[linux系统编程/函数笔记/目录与文件系统/opendir|opendir]]
