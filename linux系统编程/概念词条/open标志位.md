---
title: open标志位
tags:
  - linux
  - 系统编程
  - 概念词条
  - 文件IO
---
# open标志位

## 它是什么
- `open` 的第二个参数 `flags` 里使用的控制位。

## 常见值
- `O_RDONLY`：只读打开。
- `O_WRONLY`：只写打开。
- `O_RDWR`：读写打开。
- `O_CREAT`：不存在则创建。
- `O_TRUNC`：打开时清空文件内容。
- `O_APPEND`：追加写入。
- `O_NONBLOCK`：非阻塞打开或操作。

## 怎么理解
- 这些标志控制文件的打开方式和附加行为。
- `O_CREAT` 与 `mode` 配合使用时，会受到 `umask` 影响。

## 相关入口
- [[linux系统编程/函数笔记/文件IO/open|open]]
- [[linux系统编程/概念词条/umask|umask]]
