---
title: memset
tags:
  - linux
  - 系统编程
  - 概念词条
  - 标准库函数
---
# memset

`memset` 用来把一段内存的每个字节都设置成指定值，常用于初始化和清零。

## 常见原型

- `void *memset(void *s, int c, size_t n);`

## 你需要记住的点

- `s` 是目标内存地址。
- `c` 是要写入的字节值，实际上按字节填充。
- `n` 是填充的字节数。
- 结构体、数组初始化时经常会用到它。

## 相关概念

- [[linux系统编程/概念词条/memcpy]]
- [[linux系统编程/概念词条/memmove]]
