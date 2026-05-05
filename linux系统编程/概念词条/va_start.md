---
title: va_start
tags:
  - linux
  - 系统编程
  - 概念词条
---
# va_start

## 是什么

`va_start` 是处理可变参数时使用的宏，用来初始化 [[linux系统编程/概念词条/va_list|va_list]]。

## 依赖头文件

- `#include <stdarg.h>`

## 常见写法

```c
va_list ap;
va_start(ap, last);
```

这里的 `last` 必须是函数参数表中“省略号前最后一个固定参数”。

## 怎么理解

它的作用就是告诉程序：

- 可变参数从哪里开始
- 现在可以用 `va_arg` 去一个个取后面的参数了

## 易错点

- 第二个参数必须写成省略号前最后一个固定参数名。
- 没有 `va_start`，后面的 `va_arg` 不能正常工作。

## 相关笔记

- [[linux系统编程/概念词条/va_list|va_list]]
- [[linux系统编程/概念词条/va_arg|va_arg]]
- [[linux系统编程/概念词条/va_end|va_end]]
