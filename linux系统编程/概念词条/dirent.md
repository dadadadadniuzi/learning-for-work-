---
title: dirent
tags:
  - linux
  - 系统编程
  - 概念词条
  - 目录
---
# dirent

## 它是什么
- 表示一个目录项的结构体类型，常由 `readdir` 返回。

## 常见成员
- `d_name`：文件名。
- `d_ino`：inode 号。
- `d_type`：文件类型，部分系统可用。

## 怎么理解
- 它描述的是“目录里的一个条目”。
- `readdir` 每次给你一个 `struct dirent *`，让你逐项查看。

## 相关入口
- [[linux系统编程/函数笔记/目录与文件系统/readdir|readdir]]
- [[linux系统编程/函数笔记/目录与文件系统/opendir|opendir]]
