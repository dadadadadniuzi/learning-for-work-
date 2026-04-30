---
title: mmap保护权限
tags:
  - linux
  - 系统编程
  - 概念词条
  - mmap
---
# mmap保护权限

## 它是什么
- `mmap` 的 `prot` 参数，用来表示这段映射区能以什么方式访问。

## 常见取值
- `PROT_READ`：可读。
- `PROT_WRITE`：可写。
- `PROT_EXEC`：可执行。
- `PROT_NONE`：不可访问。

## 怎么理解
- 它描述的是“映射到进程地址空间后，能怎么访问”。
- 通常和 `MAP_*` 标志一起看。

## 相关入口
- [[linux系统编程/函数笔记/IPC/mmap|mmap]]
