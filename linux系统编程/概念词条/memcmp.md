---
title: memcmp
tags:
  - linux
  - 系统编程
  - 概念词条
  - 标准库函数
---
# memcmp

`memcmp` 用来按字节比较两段内存。

## 常见原型

- `int memcmp(const void *s1, const void *s2, size_t n);`

## 你需要记住的点

- 只比较前 `n` 个字节。
- 返回值通常表示大小关系，不是简单的真/假。
- 适合比较原始字节数据，不适合替代语义级比较。

## 相关概念

- [[linux系统编程/概念词条/memcpy]]
- [[linux系统编程/概念词条/memmove]]
- [[linux系统编程/概念词条/memset]]
