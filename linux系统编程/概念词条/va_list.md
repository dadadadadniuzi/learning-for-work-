---
title: va_list
tags:
  - linux
  - 系统编程
  - 概念词条
---
# va_list

## 是什么

`va_list` 是 C 语言里处理“可变参数”的核心类型。

当一个函数参数表里出现省略号 `...` 时，函数内部通常就要借助 `va_list` 来依次取出这些额外参数。

## 依赖头文件

- `#include <stdarg.h>`

## 怎么理解

例如下面这个函数：

```c
int my_printf(const char *fmt, ...);
```

这里 `...` 表示调用者可以继续传更多参数，但函数内部并不知道“后面到底传了多少个、分别是什么类型”。

这时就要用：

- [[linux系统编程/概念词条/va_list|va_list]]：保存可变参数读取状态
- [[linux系统编程/概念词条/va_start|va_start]]：开始读取
- [[linux系统编程/概念词条/va_arg|va_arg]]：逐个取参数
- [[linux系统编程/概念词条/va_end|va_end]]：结束读取

## 常见用法

```c
#include <stdarg.h>

int sum(int count, ...) {
    va_list ap;
    int i, total = 0;

    va_start(ap, count);
    for (i = 0; i < count; i++) {
        total += va_arg(ap, int);
    }
    va_end(ap);

    return total;
}

va_list valst;
va_start(valst, format);
int m = vsnprintf(m_buf + n, m_log_buf_size - n - 1, format, valst);
//若m_buf = "2026-05-02 14:33:22.123456 [info]: "
//设format = "client %d connected from %s";
//valst client_id = 1001; ip = "127.0.0.1";
完成后
m_buf =
"2026-05-02 14:33:22.123456 [info]: client 1001 connected from 127.0.0.1"
m = strlen("client 1001 connected from 127.0.0.1");
```

## 你要记住的点

- `va_list` 不是函数，而是一个类型。
- 它本身不负责“取值”，只是配合 `va_start` / `va_arg` / `va_end` 使用。
- 可变参数本身没有类型检查，所以调用和读取必须严格约定好类型与顺序。

## 易错点

- 没有 `va_start` 就不能直接用 `va_arg`。
- 用完后要 `va_end`。
- `va_arg` 取值时类型必须写对，否则结果可能错误甚至产生未定义行为。
- 可变参数不适合做特别复杂的接口设计，调试成本较高。

## 常见出现位置

- 自定义日志函数
- 自定义格式化输出函数
- 包装 `printf` 风格接口
- 某些带 `...` 的系统接口学习中

## 相关笔记

- [[linux系统编程/概念词条/va_start|va_start]]
- [[linux系统编程/概念词条/va_arg|va_arg]]
- [[linux系统编程/概念词条/va_end|va_end]]
- [[linux系统编程/函数笔记/进程控制/execl.md|execl]]
- [[linux系统编程/函数笔记/进程控制/execlp.md|execlp]]
