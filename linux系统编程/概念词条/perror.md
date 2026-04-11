---
title: perror
tags:
  - linux
  - 系统编程
  - 概念词条
  - 错误处理
---
# perror

## 它是什么
- 把当前 `errno` 对应的错误原因打印出来的函数。

## 函数原型
- `void perror(const char *s);`

## 怎么理解
- 它通常和失败返回值一起使用。
- 会输出你传入的前缀，再加上 `errno` 对应的系统错误描述。

## 相关入口
- [[linux系统编程/概念词条/errno|errno]]
- [[linux系统编程/函数笔记/文件IO/open|open]]
- [[linux系统编程/函数笔记/目录与文件系统/opendir|opendir]]
