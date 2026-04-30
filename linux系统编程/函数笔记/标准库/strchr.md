---
title: strchr
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/标准库
---
# strchr

> [!info] 功能
> 在字符串中查找某个字符第一次出现的位置。

## 函数原型

- `char *strchr(const char *s, int c);`

## 依赖头文件

- `#include <string.h>`

## 输入参数

- `s`
  要被扫描的源字符串。
  它必须是一个合法的、以 `\0` 结尾的 C 风格字符串。

- `c`
  要查找的字符。
  虽然参数类型写的是 `int`，但通常传一个字符常量，例如 `'a'`、`':'`、`'\n'`。

## 输出参数

- 无直接输出参数。

## 返回值

- 如果找到，返回指向第一次命中字符的指针。
- 如果找不到，返回 `NULL`。

## 怎么理解

`strchr` 是“找一个字符”。

例如：

```c
char *p = strchr("name:value", ':');
```

它会返回指向 `':'` 的指针。

## 常见用法

```c
#include <stdio.h>
#include <string.h>

int main(void) {
    char *p = strchr("hello", 'l');

    if (p != NULL) {
        printf("%c\n", *p);
    }

    return 0;
}
```

这里返回的是第一个 `l` 的位置。

## 典型场景

- 查找分隔符
- 找冒号、等号、斜杠等特殊字符
- 快速定位某个字段边界

## 易错点

- 返回的是指针，不是下标。
- 只返回第一次出现的位置，不会返回后续所有位置。
- 如果返回 `NULL`，不能继续解引用。
- `strchr` 连字符串结尾的 `\0` 也可以查找。

## 和相关函数的区别

- `strchr`：找一个指定字符。
- `strpbrk`：找字符集合中的任意一个字符。
- `strspn`：统计开头连续满足条件的长度。

## 知识点补充

- 如果想找最后一次出现的位置，常会用到 [[linux系统编程/函数笔记/标准库/strrchr|strrchr]]。
- 当你已经拿到返回指针 `p` 后，`p - s` 才能换算成下标位置。

## 相关笔记

- 函数笔记：[[linux系统编程/函数笔记/标准库/strpbrk|strpbrk]]
- 函数笔记：[[linux系统编程/函数笔记/标准库/strspn|strspn]]
- 函数笔记：[[linux系统编程/函数笔记/标准库/strcasecmp|strcasecmp]]
- 函数笔记：[[linux系统编程/函数笔记/标准库/strrchr|strrchr]]

## 相关课时

- [[linux系统编程/课时笔记/01 Linux基础与开发环境/01 Linux目录、路径与常用命令|01 Linux目录、路径与常用命令]]

## 相关模块

- [[linux系统编程/01 Linux基础与开发环境|01 Linux基础与开发环境]]
