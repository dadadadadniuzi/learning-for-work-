---
title: strcpy
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/标准库
---
# strcpy

> [!info] 功能
> 把一个以 `\0` 结尾的字符串复制到另一个字符数组中。

## 函数原型

- `char *strcpy(char *dest, const char *src);`

## 依赖头文件

- `#include <string.h>`

## 输入参数

- `dest`
  目标字符串缓冲区。
  它必须有足够大的空间来容纳 `src` 中的全部字符以及结尾的 `\0`。

- `src`
  源字符串。
  它必须是一个合法的、以 `\0` 结尾的 C 风格字符串。

## 输出参数

- 无直接输出参数。
- 复制结果会写入 `dest` 指向的缓冲区。

## 返回值

- 返回目标字符串起始地址，也就是 `dest`。

## 怎么理解

`strcpy` 不是按“指定长度”复制，而是一直复制到源字符串中的 `\0` 为止。

例如：

```c
char src[] = "hello";
char dest[32];

strcpy(dest, src);
```

执行后，`dest` 中会得到完整的 `"hello"`，并自动带上结尾的 `\0`。

## 常见用法

```c
#include <stdio.h>
#include <string.h>

int main(void) {
    char name[32];

    strcpy(name, "linux");
    printf("%s\n", name);

    return 0;
}
```

## 易错点

- `dest` 空间不够会发生缓冲区溢出。
- `strcpy` 不会检查目标空间大小。
- `src` 必须以 `\0` 结尾，否则会一直向后读，造成未定义行为。
- 源和目标内存重叠时，不应该使用 `strcpy`。

## 和相关函数的区别

- `strcpy`：复制整个字符串，直到 `\0`。
- `strcat`：把一个字符串追加到另一个字符串后面。
- `[[linux系统编程/概念词条/memcpy|memcpy]]`：按字节复制内存，不关心 `\0`。
- `[[linux系统编程/概念词条/memmove|memmove]]`：也是按字节复制，但可以处理重叠内存。

## 知识点补充

- `strcpy` 处理的是“字符串”，不是任意二进制内存。
- C 字符串的结束标志是空字符 `\0`。
- 目标数组至少要满足：`strlen(src) + 1` 的空间需求。

## 相关笔记

- 函数笔记：[[linux系统编程/函数笔记/标准库/strcat|strcat]]
- 概念词条：[[linux系统编程/概念词条/memcpy|memcpy]]
- 概念词条：[[linux系统编程/概念词条/memmove|memmove]]
- 概念词条：[[linux系统编程/概念词条/memset|memset]]

## 相关课时

- [[linux系统编程/课时笔记/01 Linux基础与开发环境/01 Linux目录、路径与常用命令|01 Linux目录、路径与常用命令]]

## 相关模块

- [[linux系统编程/01 Linux基础与开发环境|01 Linux基础与开发环境]]
