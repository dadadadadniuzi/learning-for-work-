---
title: write
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/文件IO
---
# write

> [!info] 功能
> 向[[linux系统编程/概念词条/文件描述符|文件描述符]]写入数据。

## 函数原型

- [[linux系统编程/概念词条/ssize_t|ssize_t]] write(int fd, const void \*buf, [[linux系统编程/概念词条/size_t|size_t]] count);

## 依赖头文件

- `#include <unistd.h>`

## 输入参数

- `fd`：要写入的[[linux系统编程/概念词条/文件描述符|文件描述符]]。
- `buf`：待写入数据缓冲区地址，类型是 `const void \*`，表示它只负责读取这段内存，不会修改调用者提供的数据。
- `count`：期望写入的字节数，类型是 [[linux系统编程/概念词条/size_t|size_t]]。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回实际写入的字节数，类型是 [[linux系统编程/概念词条/ssize_t|ssize_t]]。
- 失败返回 `-1`，并设置 [[linux系统编程/概念词条/errno|errno]]。

## 知识点补充

- `write` 也可能短写，不保证一次写完。
- 对普通文件、管道、套接字的写入语义并不完全相同。
- 缓冲区里的内容不会自动同步到磁盘，是否立即落盘取决于内核缓存与刷新策略。

## 常见用法

- 配合循环写入处理短写。

## 易错点

- 不要假设一次 `write` 就能写完所有数据。
- 返回值要和 `count` 比较，必要时继续写剩余部分。

## 相关概念

- [[linux系统编程/概念词条/ssize_t|ssize_t]]
- [[linux系统编程/概念词条/文件描述符|文件描述符]]
- [[linux系统编程/概念词条/errno|errno]]
- [[linux系统编程/概念词条/perror|perror]]
- [[linux系统编程/概念词条/短读短写|短读短写]]

## 相关课时

- [[linux系统编程/课时笔记/02 文件IO/02 read-write读写模型|02 read-write读写模型]]

## 相关模块

- [[linux系统编程/02 文件IO|02 文件IO]]

