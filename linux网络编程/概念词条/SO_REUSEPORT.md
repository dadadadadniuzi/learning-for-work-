---
title: SO_REUSEPORT
tags:
  - linux
  - 网络编程
  - 概念词条
---
# SO_REUSEPORT

## 它是什么

- `SO_REUSEPORT` 是 socket 端口复用相关选项，允许多个 socket 绑定到同一个 IP 和端口，并由内核进行连接或报文分配。

## 怎么理解

- 它常用于多进程或多线程服务的负载分担场景。
- 它和 [[linux网络编程/概念词条/SO_REUSEADDR|SO_REUSEADDR]] 都和复用有关，但目标和语义不同。

## 易错点

- 不同系统对 `SO_REUSEPORT` 支持和语义可能不同。
- 初学课程里更常见的是 `SO_REUSEADDR`。

## 常见出现位置

- [[linux网络编程/函数笔记/Socket/setsockopt|setsockopt]]
- [[linux网络编程/课时笔记/04 高并发服务器/03 端口复用|03 端口复用]]
