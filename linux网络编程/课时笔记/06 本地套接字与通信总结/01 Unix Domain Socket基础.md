---
title: Unix Domain Socket基础
tags:
  - linux
  - 网络编程
  - 课时笔记
  - 网络编程/本地套接字
---
# Unix Domain Socket基础

## 本节学什么

- [[linux网络编程/概念词条/Unix Domain Socket|Unix Domain Socket]]
- [[linux网络编程/概念词条/AF_UNIX与AF_LOCAL|AF_UNIX / AF_LOCAL]]
- 本地套接字和网络套接字的区别
- 本地套接字仍然使用 socket API

## 本节学什么详解

- [[linux网络编程/概念词条/Unix Domain Socket|Unix Domain Socket]]：Unix 域套接字，也叫本地套接字，用于同一台主机上的进程间通信。
- [[linux网络编程/概念词条/AF_UNIX与AF_LOCAL|AF_UNIX / AF_LOCAL]]：它们表示本地通信地址族，实际使用中通常可认为是同一类地址族。
- 本地套接字和网络套接字的区别：网络套接字通过 IP 和端口定位远程或本机服务；本地套接字通过文件系统路径或抽象地址定位同机进程通信端点。
- 本地套接字仍然使用 socket API：它不是另一套完全不同的函数，仍然使用 `socket`、`bind`、`listen`、`accept`、`connect`，只是地址族和地址结构变了。

## 知识点补充

- 本地套接字不经过网卡，也不需要 IP 地址和端口。
- 相比 TCP 本机回环通信，本地套接字更适合本机进程间通信，开销通常更小。
- 本地套接字可以使用流式语义，也可以使用数据报语义，常见练习多使用 `SOCK_STREAM`。

## 本节内容速览

- `AF_INET` 对应 IPv4 网络通信。
- `AF_UNIX` / `AF_LOCAL` 对应本地进程间通信。
- 本地套接字重点看地址结构 [[linux网络编程/概念词条/sockaddr_un|sockaddr_un]]。

## 复习时要回答

- 本地套接字和网络套接字最核心的差别是什么？
- 为什么 Unix Domain Socket 不需要 IP 和端口？
- 本地套接字使用哪些 socket API？

## 本节关键函数

- [[linux网络编程/函数笔记/Socket/socket|socket]]
- [[linux网络编程/函数笔记/Socket/bind|bind]]
- [[linux网络编程/函数笔记/Socket/connect|connect]]

## 本节关键概念

- [[linux网络编程/概念词条/Unix Domain Socket|Unix Domain Socket]]
- [[linux网络编程/概念词条/AF_UNIX与AF_LOCAL|AF_UNIX与AF_LOCAL]]
- [[linux网络编程/概念词条/sockaddr_un|sockaddr_un]]
- [[linux网络编程/概念词条/本地通信|本地通信]]

## 关联模块

- [[linux网络编程/06 本地套接字与通信总结|06 本地套接字与通信总结]]
