---
title: mmap映射标志
tags:
  - linux
  - 系统编程
  - 概念词条
  - mmap
---
# mmap映射标志

## 它是什么
- `mmap` 的 `flags` 参数，用来表示映射方式。

## 常见取值
- `MAP_SHARED`：共享映射，修改可回写到底层对象。
- `MAP_PRIVATE`：私有映射，修改通常不会影响底层对象。
- `MAP_ANONYMOUS`：匿名映射，不关联文件。

## 怎么理解
- 它决定“映射和底层对象的关系”。
- 和 `prot` 不同，`flags` 关心的是映射模式，而不是访问权限。

## 相关入口
- [[linux系统编程/函数笔记/IPC/mmap|mmap]]
