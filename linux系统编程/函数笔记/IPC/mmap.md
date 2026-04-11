---
title: mmap
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/IPC
---
# mmap

> [!info] 功能
> 把文件或匿名内存映射到进程地址空间。

## 函数原型

- void \*mmap(void \*addr, [[linux系统编程/概念词条/size_t|size_t]] length, int prot, int flags, int fd, [[linux系统编程/概念词条/off_t|off_t]] offset);

## 依赖头文件

- `#include <sys/mman.h>`
- `#include <sys/types.h>`
- `#include <sys/stat.h>`
- `#include <fcntl.h>`

## 输入参数

- `addr`：建议映射起始地址，通常传 `NULL` 让内核自行选择。
- `length`：要映射的长度。
- `prot`：页保护权限，如 [[linux系统编程/概念词条/mmap保护权限|PROT_READ]]、[[linux系统编程/概念词条/mmap保护权限|PROT_WRITE]]。
- `flags`：映射方式，如 [[linux系统编程/概念词条/mmap映射标志|MAP_SHARED]]、[[linux系统编程/概念词条/mmap映射标志|MAP_PRIVATE]]、[[linux系统编程/概念词条/mmap映射标志|MAP_ANONYMOUS]]。
- `fd`：被映射的文件描述符；匿名映射时按标志要求处理。
- `offset`：文件映射偏移，必须满足 [[linux系统编程/概念词条/页对齐|页对齐]] 要求。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回映射区首地址。
- 失败返回 [[linux系统编程/概念词条/MAP_FAILED|MAP_FAILED]]。

## 知识点补充

- `mmap` 会把一段文件或匿名内存直接映射到进程虚拟地址空间。
- `prot` 决定“能不能读写执行”，`flags` 决定“共享还是私有、有没有文件、是否匿名”。
- 如果是文件映射，`offset` 必须页对齐，否则会失败。
- 出错时返回的不是 `NULL`，而是 [[linux系统编程/概念词条/MAP_FAILED|MAP_FAILED]]。

## 常见用法

- 文件到内存的映射访问。
- 共享内存通信。

## 易错点

- `PROT_*` 和 `MAP_*` 的语义不同，不要混为一谈。
- 判断失败要和 [[linux系统编程/概念词条/MAP_FAILED|MAP_FAILED]] 比较，而不是和 `NULL` 比较。

## 相关概念

- [[linux系统编程/概念词条/mmap保护权限|mmap保护权限]]
- [[linux系统编程/概念词条/mmap映射标志|mmap映射标志]]
- [[linux系统编程/概念词条/MAP_FAILED|MAP_FAILED]]
- [[linux系统编程/概念词条/页对齐|页对齐]]
- [[linux系统编程/概念词条/off_t|off_t]]
- [[linux系统编程/概念词条/文件描述符|文件描述符]]

## 相关课时

- [[linux系统编程/课时笔记/07 进程间通信 IPC/03 mmap内存映射|03 mmap内存映射]]

## 相关模块

- [[linux系统编程/06 进程间通信 IPC|06 进程间通信 IPC]]
