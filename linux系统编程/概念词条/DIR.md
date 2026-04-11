---
title: DIR
tags:
  - linux
  - 系统编程
  - 概念词条
  - 目录
---
# DIR

## 它是什么
- 目录流类型，不是普通文件描述符。
- 用于 `opendir` / `readdir` / `closedir` 这一套接口。

## 怎么理解
- `DIR *` 是目录遍历句柄。
- 它代表的是目录读取状态，而不是目录内容本身。

## 相关入口
- [[linux系统编程/函数笔记/目录与文件系统/opendir|opendir]]
- [[linux系统编程/函数笔记/目录与文件系统/readdir|readdir]]
- [[linux系统编程/函数笔记/目录与文件系统/closedir|closedir]]
