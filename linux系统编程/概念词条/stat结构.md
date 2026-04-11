---
title: stat结构
tags:
  - linux
  - 系统编程
  - 概念词条
  - 文件IO
---
# stat结构

## 它是什么
- `stat` / `lstat` / `fstat` 返回的文件状态结构体。

## 常见成员
- `st_mode`：文件类型和权限。
- `st_size`：文件大小。
- `st_ino`：inode 号。
- `st_nlink`：链接计数。

## 怎么理解
- 它是“文件体检报告”。
- `stat` 更关注文件元信息，不读取文件内容本身。

## 相关入口
- [[linux系统编程/函数笔记/文件IO/stat|stat]]
- [[linux系统编程/函数笔记/文件IO/lstat|lstat]]
