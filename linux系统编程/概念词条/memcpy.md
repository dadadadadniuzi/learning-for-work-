---
title: memcpy
tags:
  - linux
  - 系统编程
  - 概念词条
  - 标准库函数
---
# memcpy

`memcpy` 是 C 标准库里的内存拷贝函数，用来把一段内存中的字节复制到另一段内存中。

## 常见原型

- `void *memcpy(void *dest, const void *src, size_t n);`

## 你需要记住的点

- `dest` 是目标地址，`src` 是源地址。
- `n` 是要复制的字节数，不是元素个数。
- 它是按字节拷贝，不理解对象语义，使用时要保证内存区域合法。
- 源和目标重叠时不要用它，应考虑 [[linux系统编程/概念词条/memmove|memmove]]。

## 常见搭配

- 清零或初始化常用 [[linux系统编程/概念词条/memset|memset]]。
- 比较两段内存是否相等常用 [[linux系统编程/概念词条/memcmp|memcmp]]。

## 相关概念

- [[linux系统编程/概念词条/memmove]]
- [[linux系统编程/概念词条/memset]]
- [[linux系统编程/概念词条/memcmp]]
