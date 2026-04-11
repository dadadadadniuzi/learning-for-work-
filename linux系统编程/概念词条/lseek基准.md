---
title: lseek基准
tags:
  - linux
  - 系统编程
  - 概念词条
  - 文件IO
---
# lseek基准

## 它是什么

- `lseek` 的 `whence` 参数决定偏移量从哪里开始算。

## 常见取值

- `SEEK_SET`：从文件开头算。
- `SEEK_CUR`：从当前偏移位置算。
- `SEEK_END`：从文件末尾算。

## 你需要记住的点

- `SEEK_SET` 是最常用的绝对定位。
- `SEEK_END` 常用于获取文件长度或倒着定位。
- `lseek` 不能用于管道和套接字这类不支持随机访问的对象。

## 相关入口

- [[lseek]]
- [[文件描述符]]
- [[open标志位]]

