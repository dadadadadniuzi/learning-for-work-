---
title: strncasecmp
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/标准库
---
# strncasecmp

> [!info] 功能
> 忽略字母大小写，比较两个字符串的前 `n` 个字符。

## 函数原型

- `int strncasecmp(const char *s1, const char *s2, size_t n);`

## 依赖头文件

- `#include <strings.h>`

## 输入参数

- `s1`
  第一个要比较的字符串。

- `s2`
  第二个要比较的字符串。

- `n`
  最多比较的字符数。
  当比较到 `n` 个字符后，即使后面还有内容，也会停止。

## 输出参数

- 无直接输出参数。

## 返回值

- 如果忽略大小写后前 `n` 个字符相等，返回 `0`。
- 如果 `s1` 小于 `s2`，返回小于 `0` 的值。
- 如果 `s1` 大于 `s2`，返回大于 `0` 的值。

## 怎么理解

它和 `strcasecmp` 的区别是：`strncasecmp` 不一定比较整个字符串，而是最多比较前 `n` 个字符。

例如：

```c
strncasecmp("Content-Length", "content-type", 7);
```

这里只比较前 7 个字符，所以比较的是 `"Content"` 和 `"content"`，结果会相等。

## 常见用法

```c
#include <stdio.h>
#include <strings.h>

int main(void) {
    if (strncasecmp("HelloWorld", "helloKitty", 5) == 0) {
        printf("prefix same\n");
    }
    return 0;
}
```

## 典型场景

- 比较固定前缀
- 判断请求头字段名
- 判断字符串开头是不是某个关键字

## 易错点

- `n` 表示“最多比较多少个字符”，不是“必须正好比较这么多”。
- 如果前面已经遇到 `\0`，比较也会提前结束。
- 它返回的也不是固定的 `1`、`0`、`-1`。

## 和相关函数的区别

- `strcasecmp`：比较整个字符串。
- `strncasecmp`：只比较前 `n` 个字符。
- `strspn`：统计开头连续满足条件的长度。

## 知识点补充

- 如果你要比较协议字段、命令前缀，这个函数很常见。
- 它特别适合“只关心前缀是否匹配，而且大小写不敏感”的场景。

## 相关笔记

- 函数笔记：[[linux系统编程/函数笔记/标准库/strcasecmp|strcasecmp]]
- 函数笔记：[[linux系统编程/函数笔记/标准库/strspn|strspn]]
- 概念词条：[[linux系统编程/概念词条/size_t|size_t]]

## 相关课时

- [[linux系统编程/课时笔记/01 Linux基础与开发环境/01 Linux目录、路径与常用命令|01 Linux目录、路径与常用命令]]

## 相关模块

- [[linux系统编程/01 Linux基础与开发环境|01 Linux基础与开发环境]]
