---
title: snprintf
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/标准库
---
# snprintf

> [!info] 功能
> 按格式把数据写入字符缓冲区，并且带有长度上限控制，常用于安全地拼接字符串。

## 函数原型

- `int snprintf(char *str, size_t size, const char *format, ...);`

## 依赖头文件

- `#include <stdio.h>`

## 输入参数

- `str`
  目标字符缓冲区首地址。
  `snprintf` 会把格式化后的结果写到这里。

- `size`
  目标缓冲区总大小，类型是 [[linux系统编程/概念词条/size_t|size_t]]。
  它表示这块缓冲区最多能容纳多少字节，包括最后的 `\0`。

- `format`
  格式字符串。
  写法和 `printf` 类似，例如：
  - `"%d"`
  - `"%s:%d"`
  - `"user=%s, score=%d"`

- `...`
  可变参数列表，对应 `format` 中的格式占位符。
  它和 [[linux系统编程/概念词条/va_list|va_list]] 这类可变参数机制属于同一条知识线，只不过 `snprintf` 作为调用者接口，不需要你手动写 `va_start` / `va_arg` / `va_end`。

## 输出参数

- `str`
  保存格式化后的输出字符串。
  如果空间足够，它会得到完整内容并带有结尾 `\0`。
  如果空间不足，结果会被截断，但通常仍会尽量保证以 `\0` 结束。

## 返回值

- 返回“本来想写出的字符数”，不包括结尾 `\0`。
- 如果返回值小于 `size`，说明缓冲区足够，内容完整写入了。
- 如果返回值大于或等于 `size`，说明发生了截断。
- 出错时返回负值。

## 怎么理解

`snprintf` 可以理解成“带长度保护的 `sprintf`”。

它最大的价值不是格式化本身，而是：

- 你可以告诉它目标缓冲区有多大
- 它不会像 `sprintf` 那样毫无边界地往后写

所以它特别适合：

- 拼路径
- 拼日志文本
- 拼协议报文
- 拼错误提示字符串

## 常见用法

```c
#include <stdio.h>

int main(void) {
    char buf[64];
    int n = snprintf(buf, sizeof(buf), "port=%d, ip=%s", 8080, "127.0.0.1");

    printf("n=%d, buf=%s\n", n, buf);
    return 0;
}
```

## 典型场景

- 构造文件路径
- 生成日志文本
- 生成响应头或命令字符串
- 把数字和字符串安全拼接到缓冲区

## 易错点

- `size` 是缓冲区总大小，不是“最多可写字符数”那么简单，它还要考虑结尾 `\0`。
- 返回值是“期望写出的完整长度”，不是“实际放进缓冲区的长度”。
- 不能只看 `buf` 有内容就认为没截断，应该检查返回值是否 `>= size`。
- 它虽然更安全，但如果你传错 `format` 和参数类型，仍然会出问题。

## 和相关函数的区别

- `sprintf`：格式化写入字符串，但不限制长度，容易溢出。
- `snprintf`：格式化写入字符串，并限制最大写入范围。
- `vsnprintf`：和 `snprintf` 类似，但参数来源是 [[linux系统编程/概念词条/va_list|va_list]]。

## 知识点补充

- 现代 C/C++ 代码里，很多场景会优先选择 `snprintf` 而不是 `sprintf`。
- 如果你在写带 `...` 的自定义日志函数，内部通常就会进一步调用 `vsnprintf`。
- 你在 TinyWebServer 这类项目里看到组装日志、拼响应文本时，`snprintf` 和 `vsnprintf` 都很常见。

## 相关笔记

- 概念词条：[[linux系统编程/概念词条/size_t|size_t]]
- 概念词条：[[linux系统编程/概念词条/va_list|va_list]]
- 概念词条：[[linux系统编程/概念词条/va_start|va_start]]
- 概念词条：[[linux系统编程/概念词条/va_arg|va_arg]]
- 概念词条：[[linux系统编程/概念词条/va_end|va_end]]
- 函数笔记：[[linux系统编程/函数笔记/标准库/strcat|strcat]]
- 函数笔记：[[linux系统编程/函数笔记/标准库/strcpy|strcpy]]

## 相关课时

- [[linux系统编程/课时笔记/01 Linux基础与开发环境/01 Linux目录、路径与常用命令|01 Linux目录、路径与常用命令]]

## 相关模块

- [[linux系统编程/01 Linux基础与开发环境|01 Linux基础与开发环境]]
