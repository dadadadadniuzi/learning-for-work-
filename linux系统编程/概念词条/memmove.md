---
title: memmove
tags:
  - linux
  - 系统编程
  - 概念词条
  - 标准库函数
---
# memmove

`memmove` 也是内存拷贝函数，但它能安全处理源和目标内存重叠的情况。

## 常见原型

- `void *memmove(void *dest, const void *src, size_t n);`

## 你需要记住的点

- 语义和 `memcpy` 很像，都是复制字节。
- 最大区别是它支持重叠内存。
- 不确定是否重叠时，优先考虑它。

## 相关概念

- [[linux系统编程/概念词条/memcpy]]
- [[linux系统编程/概念词条/memset]]
