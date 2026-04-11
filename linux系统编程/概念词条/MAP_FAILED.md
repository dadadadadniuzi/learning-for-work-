---
title: MAP_FAILED
tags:
  - linux
  - 系统编程
  - 概念词条
  - mmap
---
# MAP_FAILED

## 它是什么
- `mmap` 失败时返回的专用失败值。

## 怎么理解
- 它不是 `NULL`。
- 判断 `mmap` 是否成功，要和 `MAP_FAILED` 比较。

## 相关入口
- [[linux系统编程/函数笔记/IPC/mmap|mmap]]
- [[linux系统编程/函数笔记/IPC/munmap|munmap]]
