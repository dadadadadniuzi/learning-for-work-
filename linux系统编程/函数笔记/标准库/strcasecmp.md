---
title: strcasecmp
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/标准库
---
# strcasecmp

> [!info] 功能
> 忽略字母大小写比较两个字符串。

## 函数原型

- `int strcasecmp(const char *s1, const char *s2);`

## 依赖头文件

- `#include <strings.h>`

## 输入参数

- `s1`
  第一个要比较的字符串。
  它必须是一个合法的、以 `\0` 结尾的 C 风格字符串。

- `s2`
  第二个要比较的字符串。
  它也必须是一个合法的、以 `\0` 结尾的 C 风格字符串。

## 输出参数

- 无直接输出参数。

## 返回值

- 如果忽略大小写后两个字符串相等，返回 `0`。
- 如果 `s1` 小于 `s2`，返回小于 `0` 的值。
- 如果 `s1` 大于 `s2`，返回大于 `0` 的值。

## 怎么理解

`strcasecmp` 和普通字符串比较函数的区别在于：

- `"Hello"` 和 `"hello"` 会被认为相等
- `"ABC"` 和 `"abc"` 也会被认为相等

也就是说，它比较时不区分大小写。

## 常见用法

```c
#include <stdio.h>
#include <strings.h>

int main(void) {
    if (strcasecmp("GET", "get") == 0) {
        printf("same\n");
    }
    return 0;
}
```

## 典型场景

- 比较 HTTP 方法名
- 比较命令字
- 处理大小写不敏感的配置项

## 易错点

- 它不是标准 C 的 `<string.h>` 函数，通常在 `<strings.h>` 中声明。
- 返回值不是单纯的 `1`、`0`、`-1`，只需要判断正负和是否为 `0`。
- 它比较的是字符串内容，不会修改原字符串。

## 和相关函数的区别

- `strcasecmp`：忽略大小写比较整个字符串。
- `strncasecmp`：忽略大小写比较前 `n` 个字符。
- `strcpy`：复制字符串。
- `strcat`：拼接字符串。

## 知识点补充

- 如果你只关心“是否相等”，最常见的写法就是判断 `== 0`。
- 网络协议、命令行选项、配置字段里经常会需要大小写不敏感比较。

## 相关笔记

- 函数笔记：[[linux系统编程/函数笔记/标准库/strncasecmp|strncasecmp]]
- 函数笔记：[[linux系统编程/函数笔记/标准库/strchr|strchr]]
- 函数笔记：[[linux系统编程/函数笔记/标准库/strcpy|strcpy]]

## 相关课时

- [[linux系统编程/课时笔记/01 Linux基础与开发环境/01 Linux目录、路径与常用命令|01 Linux目录、路径与常用命令]]

## 相关模块

- [[linux系统编程/01 Linux基础与开发环境|01 Linux基础与开发环境]]
