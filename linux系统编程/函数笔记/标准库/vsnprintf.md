---
title: vsnprintf
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/标准库
---
# vsnprintf

> [!info] 功能
> 按格式把数据写入字符缓冲区，并带有长度上限控制；它是 `snprintf` 的 `va_list` 版本，常用于自定义日志函数、封装格式化输出函数。

## 函数原型

- `int vsnprintf(char *str, size_t size, const char *format, va_list ap);`

## 依赖头文件

- `#include <stdio.h>`
- `#include <stdarg.h>`

## 输入参数

- `str`
  目标缓冲区首地址。格式化后的输出会写到这里。

- `size`
  目标缓冲区总大小，类型是 [[linux系统编程/概念词条/size_t|size_t]]。它表示这块缓冲区最多能容纳多少字节，包括最后的 `\0`。

- `format`
  格式字符串，写法和 `printf` / `snprintf` 一样，比如：
  - `"%d"`
  - `"%s:%d"`
  - `"user=%s, score=%d"`

- `ap`
  一个 [[linux系统编程/概念词条/va_list|va_list]] 类型的可变参数对象。
  它通常不是直接手写出来的，而是通过：
  - [[linux系统编程/概念词条/va_start|va_start]]
  - [[linux系统编程/概念词条/va_arg|va_arg]]
  - [[linux系统编程/概念词条/va_end|va_end]]
  这一套机制配合使用。

## 输出参数

- `str`
  保存格式化后的输出结果。如果空间足够，结果会完整写入并带结尾 `\0`；如果空间不够，结果会被截断。

## 返回值

- 返回“本来想写出的字符数”，不包括最后的 `\0`
- 如果返回值小于 `size`，说明缓冲区足够
- 如果返回值大于或等于 `size`，说明结果被截断了
- 出错时返回负值

## 怎么理解

`vsnprintf` 和 `snprintf` 的关系可以这样记：

- `snprintf`：你直接把后面的参数一个个传进去
- `vsnprintf`：你已经把这些参数收集成了一个 `va_list`，再交给它处理

所以它特别适合写这种“再包装一层”的函数：

```c
void mylog(const char *fmt, ...)
```

这种时候，外层函数收到的是 `...`，内部通常就会把它转成 `va_list`，再调用 `vsnprintf`。

## 常见用法

```c
#include <stdio.h>
#include <stdarg.h>

void format_text(char *buf, size_t size, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, size, fmt, ap);
    va_end(ap);
}
```

## 典型场景

- 自定义日志函数
- 自定义错误输出函数
- 对 `printf` 风格接口再封装一层
- 项目里统一格式化文本输出

## 易错点

- `ap` 必须是一个已经正确初始化好的 [[linux系统编程/概念词条/va_list|va_list]]
- 用完后通常要配对调用 [[linux系统编程/概念词条/va_end|va_end]]
- 返回值表示“理想完整长度”，不是“实际写入缓冲区的长度”
- 和 `snprintf` 一样，也要检查是否发生了截断

## 和相关函数的区别

- `sprintf`：格式化写字符串，但不限制长度，容易越界
- `snprintf`：格式化写字符串，并限制最大写入范围
- `vsnprintf`：和 `snprintf` 类似，但参数来源是 [[linux系统编程/概念词条/va_list|va_list]]

## 知识点补充

- 只要你在代码里看到函数参数里带 `...`，通常就和 `va_list` / `vsnprintf` 这条知识线有关
- 许多日志模块内部并不会直接把 `...` 一路传来传去，而是尽快转成 `va_list` 再交给 `vsnprintf`
- `vsnprintf` 在项目代码阅读中比单纯的 `printf` 更值得注意，因为它通常出现在“封装层”

## 相关笔记

- 概念词条：[[linux系统编程/概念词条/size_t|size_t]]
- 概念词条：[[linux系统编程/概念词条/va_list|va_list]]
- 概念词条：[[linux系统编程/概念词条/va_start|va_start]]
- 概念词条：[[linux系统编程/概念词条/va_arg|va_arg]]
- 概念词条：[[linux系统编程/概念词条/va_end|va_end]]
- 函数笔记：[[linux系统编程/函数笔记/标准库/snprintf|snprintf]]

## 相关课时

- [[linux系统编程/课时笔记/01 Linux基础与开发环境/01 Linux目录、路径与常用命令|01 Linux目录、路径与常用命令]]

## 相关模块

- [[linux系统编程/01 Linux基础与开发环境|01 Linux基础与开发环境]]
