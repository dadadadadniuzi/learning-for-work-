---
title: optarg
tags:
  - linux
  - 系统编程
  - 概念词条
---
# optarg

## 是什么

`optarg` 是 `getopt` 使用的一个全局变量，用来保存“当前选项对应的参数字符串”。

## 头文件

- 通常配合 `#include <unistd.h>` 使用

## 怎么理解

如果命令行里有：

```bash
./app -p 8080
```

而 `optstring` 写成：

```c
"p:"
```

那么当 `getopt` 解析到 `-p` 时：

- 返回值是 `'p'`
- `optarg` 指向字符串 `"8080"`

## 常见场景

- 读取端口号
- 读取配置文件路径
- 读取线程数量

## 相关笔记

- [[linux系统编程/函数笔记/标准库/getopt|getopt]]
- [[linux系统编程/概念词条/optind|optind]]
- [[linux系统编程/概念词条/optopt|optopt]]
