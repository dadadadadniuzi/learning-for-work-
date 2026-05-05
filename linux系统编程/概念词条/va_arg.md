---
title: va_arg
tags:
  - linux
  - 系统编程
  - 概念词条
---
# va_arg

## 是什么

`va_arg` 是处理可变参数时使用的宏，用来从 [[linux系统编程/概念词条/va_list|va_list]] 中取出下一个参数。

## 依赖头文件

- `#include <stdarg.h>`

## 常见写法

```c
int x = va_arg(ap, int);
```

## 怎么理解

它每调用一次，就会：

- 按你指定的类型取出当前参数
- 然后把读取位置移动到下一个参数

所以它通常写在循环里，按顺序一个个读取。

## 易错点

- 第二个参数的类型必须写对。
- 如果实际传的是 `double`，你却按 `int` 取，结果就会错。
- 取值顺序必须和调用者传参顺序一致。

## 相关笔记

- [[linux系统编程/概念词条/va_list|va_list]]
- [[linux系统编程/概念词条/va_start|va_start]]
- [[linux系统编程/概念词条/va_end|va_end]]
