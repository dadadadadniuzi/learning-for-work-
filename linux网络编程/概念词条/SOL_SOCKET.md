---
title: SOL_SOCKET
tags:
  - linux
  - 网络编程
  - 概念词条
---
# SOL_SOCKET

## 它是什么

- `SOL_SOCKET` 是 `setsockopt` / `getsockopt` 中的选项层级，表示设置通用 socket 层选项。

## 怎么理解

- `setsockopt` 的 `level` 参数用来说明选项属于哪一层。
- 端口复用这类通用 socket 选项通常使用 `SOL_SOCKET`。

## 常见搭配

- `SOL_SOCKET + SO_REUSEADDR`
- `SOL_SOCKET + SO_REUSEPORT`

## 常见出现位置

- [[linux网络编程/函数笔记/Socket/setsockopt|setsockopt]]
