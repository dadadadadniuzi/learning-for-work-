---
title: mode_t
tags:
  - linux
  - 系统编程
  - 概念词条
  - 文件IO
---
# mode_t

## 它是什么
- 表示文件权限和文件类型相关位的类型。

## 怎么理解
- 在 `open(..., mode)`、`mkfifo`、`chmod`、`umask` 这些接口里很常见。
- 它经常配合八进制权限值一起出现，比如 `0644`、`0755`。

## 相关入口
- [[linux系统编程/函数笔记/文件IO/open|open]]
- [[linux系统编程/函数笔记/IPC/mkfifo|mkfifo]]
- [[linux系统编程/概念词条/umask|umask]]
