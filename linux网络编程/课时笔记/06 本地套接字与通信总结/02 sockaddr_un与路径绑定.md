---
title: sockaddr_un与路径绑定
tags:
  - linux
  - 网络编程
  - 课时笔记
  - 网络编程/本地套接字
---
# sockaddr_un与路径绑定

## 本节学什么

- [[linux网络编程/概念词条/sockaddr_un|sockaddr_un]]
- [[linux网络编程/概念词条/sun_path|sun_path]]
- [[linux网络编程/概念词条/路径绑定|路径绑定]]
- 为什么常在 `bind` 前调用 `unlink`

## 本节学什么详解

- [[linux网络编程/概念词条/sockaddr_un|sockaddr_un]]：Unix Domain Socket 使用的地址结构，作用类似 IPv4 中的 `sockaddr_in`。
- [[linux网络编程/概念词条/sun_path|sun_path]]：`sockaddr_un` 中保存路径名的字段，用来标识本地 socket 通信端点。
- [[linux网络编程/概念词条/路径绑定|路径绑定]]：服务器端把本地 socket 绑定到一个文件系统路径，客户端通过同一路径找到服务器。
- `unlink`：如果上一次运行残留了同名 socket 文件，再次 `bind` 可能失败，所以服务端常在绑定前删除旧路径。

## 知识点补充

- `sockaddr_un.sun_family` 通常填 `AF_UNIX` 或 `AF_LOCAL`。
- `sun_path` 是路径字符串，长度有限，不能随意写超长路径。
- socket 文件只是本地套接字的名字入口，不是普通数据文件。
- 程序退出后最好清理 socket 路径，避免下次运行冲突。

## 本节内容速览

- 准备 `struct sockaddr_un`。
- 设置 `sun_family = AF_UNIX`。
- 设置 `sun_path` 为本地路径。
- 服务器 `bind` 前常先 `unlink(path)`。
- 调用 `bind/connect` 时转换为 `struct sockaddr *`。

## 复习时要回答

- `sockaddr_un` 为什么和文件路径有关？
- `sun_path` 字段保存什么？
- 为什么本地套接字服务器经常先 `unlink`？
- `sockaddr_un` 和 `sockaddr_in` 的核心区别是什么？

## 本节关键函数

- [[linux网络编程/函数笔记/Socket/bind|bind]]
- [[linux网络编程/函数笔记/Socket/connect|connect]]
- [[linux系统编程/函数笔记/目录与文件系统/unlink|unlink]]

## 本节关键概念

- [[linux网络编程/概念词条/sockaddr_un|sockaddr_un]]
- [[linux网络编程/概念词条/sun_path|sun_path]]
- [[linux网络编程/概念词条/路径绑定|路径绑定]]
- [[linux网络编程/概念词条/AF_UNIX与AF_LOCAL|AF_UNIX与AF_LOCAL]]

## 关联模块

- [[linux网络编程/06 本地套接字与通信总结|06 本地套接字与通信总结]]
