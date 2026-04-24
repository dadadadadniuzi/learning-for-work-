---
title: Unix Domain Socket
tags:
  - linux
  - 网络编程
  - 概念词条
---
# Unix Domain Socket

## 它是什么

- Unix Domain Socket，也叫 Unix 域套接字或本地套接字，是用于同一台主机内进程间通信的 socket。
- 它不通过 IP 和端口定位通信端点，而是常通过文件系统路径定位。

## 怎么理解

- 它属于[[linux网络编程/概念词条/本地通信|本地通信]]的一种方式。
- 它的编程接口和网络 socket 很像，仍然使用 `socket`、`bind`、`listen`、`accept`、`connect`。
- 它的地址族不是 `AF_INET`，而是 [[linux网络编程/概念词条/AF_UNIX与AF_LOCAL|AF_UNIX / AF_LOCAL]]。

## 和 TCP 本机回环的区别

- TCP 回环使用 IP 和端口，仍走网络协议栈路径。
- Unix Domain Socket 使用本地地址结构，适合同机进程间通信。
- 同机通信时，Unix Domain Socket 通常更直接，也更符合 IPC 语义。
- 更完整的横向比较见 [[linux网络编程/概念词条/本地套接字和网络套接字对比|本地套接字和网络套接字对比]]。

## 常见流程

- 服务端：`socket(AF_UNIX, SOCK_STREAM, 0)`。
- 服务端：准备 [[linux网络编程/概念词条/sockaddr_un|sockaddr_un]]，设置路径。
- 服务端：`unlink(path)` 清理旧路径。
- 服务端：`bind -> listen -> accept`。
- 客户端：准备相同路径并 `connect`。

## 易错点

- socket 文件路径残留会导致下次 `bind` 失败。
- `sun_path` 长度有限，路径不要过长。
- 它不能跨主机通信。

## 常见出现位置

- [[linux网络编程/06 本地套接字与通信总结|06 本地套接字与通信总结]]
- [[linux网络编程/课时笔记/06 本地套接字与通信总结/01 Unix Domain Socket基础|01 Unix Domain Socket基础]]
- [[linux网络编程/概念词条/本地套接字和网络套接字对比|本地套接字和网络套接字对比]]
