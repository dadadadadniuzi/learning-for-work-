---
title: munmap
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/IPC
---
# munmap

> [!info] 功能
> 解除内存映射。

## 函数原型

- `int munmap(void *addr, size_t length);`

## 依赖头文件

- `#include <sys/mman.h>`

## 输入参数

- `addr`：映射区域首地址。
- `length`：映射长度。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回 `0`。
- 失败返回 `-1`，并设置 `[[linux系统编程/概念词条/errno|errno]]`。

## 知识点补充

- `munmap` 用于释放 `[[linux系统编程/函数笔记/IPC/mmap.md|mmap]]` 创建的映射区。
- 映射区解除后，原地址就不应该再访问。

## 常见用法

- `mmap` 用完后的资源回收。

## 易错点

- 不要再访问已经 `munmap` 的地址。

## 相关概念

- [[linux系统编程/概念词条/页对齐|页对齐]]
- [[linux系统编程/概念词条/MAP_FAILED|MAP_FAILED]]

## 相关课时

- [[linux系统编程/课时笔记/07 进程间通信 IPC/03 mmap内存映射|03 mmap内存映射]]

## 相关模块

- [[linux系统编程/06 进程间通信 IPC|06 进程间通信 IPC]]
