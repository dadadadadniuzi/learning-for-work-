---
title: optind
tags:
  - linux
  - 系统编程
  - 概念词条
---
# optind

## 是什么

`optind` 是 `getopt` 使用的一个全局变量，表示“下一次将要处理的 `argv` 下标”。

## 头文件

- 通常配合 `#include <unistd.h>` 使用

## 怎么理解

在 `getopt` 不断解析参数的过程中，`optind` 会随着扫描位置不断向后移动。

当选项都处理完以后：

- `optind` 往往指向第一个“不是选项的普通参数”

例如：

```bash
./app -p 8080 input.txt
```

如果 `-p 8080` 已经被解析完，那么 `optind` 很可能正好指向 `input.txt`。

## 有什么用

- 继续处理剩余普通参数
- 判断哪些参数已经被 `getopt` 消费掉了

## 相关笔记

- [[linux系统编程/函数笔记/标准库/getopt|getopt]]
- [[linux系统编程/概念词条/optarg|optarg]]
- [[linux系统编程/概念词条/optopt|optopt]]
