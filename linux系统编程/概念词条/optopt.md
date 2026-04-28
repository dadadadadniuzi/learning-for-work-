---
title: optopt
tags:
  - linux
  - 系统编程
  - 概念词条
---
# optopt

## 是什么

`optopt` 是 `getopt` 使用的一个全局变量，通常在解析出错时用来表示“出问题的那个选项字符”。

## 头文件

- 通常配合 `#include <unistd.h>` 使用

## 怎么理解

如果程序只允许：

```c
"ab"
```

但用户输入了：

```bash
./app -x
```

那么：

- `getopt` 通常返回 `?`
- `optopt` 往往就是 `'x'`

这样程序就知道到底是哪个选项非法。

## 常见用途

- 打印错误提示
- 告诉用户哪个选项不合法
- 调试命令行参数解析问题

## 相关笔记

- [[linux系统编程/函数笔记/标准库/getopt|getopt]]
- [[linux系统编程/概念词条/optarg|optarg]]
- [[linux系统编程/概念词条/optind|optind]]
