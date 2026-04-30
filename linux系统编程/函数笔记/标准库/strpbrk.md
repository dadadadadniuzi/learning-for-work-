---
title: strpbrk
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/标准库
---
# strpbrk

> [!info] 功能
> 在一个字符串中查找“是否出现了指定字符集合里的任意一个字符”，并返回第一次命中的位置。

## 函数原型

- `char *strpbrk(const char *s, const char *accept);`

## 依赖头文件

- `#include <string.h>`

## 输入参数

- `s`
  要被扫描的源字符串。
  它必须是一个合法的、以 `\0` 结尾的 C 风格字符串。

- `accept`
  候选字符集合。
  `strpbrk` 会在 `s` 里从前往后找，看看有没有字符属于这个集合。
  它也必须是一个合法的、以 `\0` 结尾的字符串。

例如：

- `accept = "aeiou"`：表示查找第一个元音字母
- `accept = ",.;"`：表示查找第一个逗号、句号或分号
- `accept = " \t\n"`：表示查找第一个空白字符

## 输出参数

- 无直接输出参数。

## 返回值

- 如果找到，返回指向 `s` 中第一次命中的那个字符的指针。
- 如果找不到，返回 `NULL`。

## 怎么理解

`strpbrk` 不是找“一个完整子串”，而是找“一组候选字符里任意一个字符”。

例如：

```c
char *p = strpbrk("linux,unix", ",;");
```

它会返回指向 `','` 的指针，因为逗号是 `",;"` 这个字符集合里的第一个命中字符。

## 常见用法

```c
#include <stdio.h>
#include <string.h>

int main(void) {
    char *p = strpbrk("hello world", "aeiou");

    if (p != NULL) {
        printf("%c\n", *p);
    }

    return 0;
}
```

这里会输出 `e`，因为它是 `"hello world"` 中第一个出现在 `"aeiou"` 里的字符。

## 典型场景

- 查找第一个分隔符
- 查找第一个空白字符
- 判断字符串里是否出现了某类特殊字符
- 做简单的词法切分前定位边界

## 易错点

- `strpbrk` 返回的是指针，不是下标。
- 如果返回 `NULL`，不能继续解引用。
- `accept` 表示“字符集合”，不是一个必须连续匹配的子串。
- 它找到的是第一次命中的字符，不会返回所有命中位置。

## 和相关函数的区别

- `strpbrk`：找“字符集合中的任意一个字符”。
- `strcpy`：复制字符串。
- `strcat`：拼接字符串。
- `[[linux系统编程/概念词条/memcpy|memcpy]]`：复制字节内存，不负责字符串查找。

## 知识点补充

- 这个函数名可以拆开理解成 string pointer break。
- 可以把它理解为“扫描到第一个属于某个集合的字符就停下”。
- 在处理配置文本、命令行、简单分词时很常见。

## 相关笔记

- 函数笔记：[[linux系统编程/函数笔记/标准库/strcpy|strcpy]]
- 函数笔记：[[linux系统编程/函数笔记/标准库/strcat|strcat]]
- 概念词条：[[linux系统编程/概念词条/memcpy|memcpy]]
- 概念词条：[[linux系统编程/概念词条/memmove|memmove]]

## 相关课时

- [[linux系统编程/课时笔记/01 Linux基础与开发环境/01 Linux目录、路径与常用命令|01 Linux目录、路径与常用命令]]

## 相关模块

- [[linux系统编程/01 Linux基础与开发环境|01 Linux基础与开发环境]]
