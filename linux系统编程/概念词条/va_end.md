---
title: va_end
tags:
  - linux
  - 系统编程
  - 概念词条
---
# va_end

## 是什么

`va_end` 是处理可变参数时使用的宏，用来结束一次 [[linux系统编程/概念词条/va_list|va_list]] 的使用。

## 依赖头文件

- `#include <stdarg.h>`

## 常见写法

```c
va_end(ap);
```

## 怎么理解

它表示这次可变参数读取流程结束了。

通常写法是：

1. `va_start`
2. 多次 `va_arg`
3. `va_end`

## 易错点

- 用完 `va_list` 后要记得 `va_end`。
- 不要在 `va_end` 之后继续使用同一个 `va_list`。

## 相关笔记

- [[linux系统编程/概念词条/va_list|va_list]]
- [[linux系统编程/概念词条/va_start|va_start]]
- [[linux系统编程/概念词条/va_arg|va_arg]]
