---
title: TCP通信基础案例
tags:
  - linux
  - 网络编程
  - 课时笔记
  - 网络编程/TCP通信
---
# TCP通信基础案例

## 本节学什么

- TCP 客户端与服务器基本案例
- 小写转大写服务器的处理流程
- 连接建立后的读写流程
- 使用 [[linux网络编程/指令查询/命令卡/nc|nc]] 测试 TCP 服务

## 本节学什么详解

- TCP 客户端与服务器基本案例：服务器先创建监听 socket，绑定 IP 和端口，进入监听状态，然后通过 `accept` 等待客户端；客户端创建 socket 后使用 `connect` 主动连接服务器。
- 小写转大写服务器的处理流程：服务器收到客户端发来的字符串后，对每个字符调用转换逻辑，例如 `toupper`，再把处理后的数据写回客户端。
- 连接建立后的读写流程：连接建立后，服务器和客户端都围绕已连接 socket 读写数据。服务器端通常使用 `cfd`，客户端通常使用 `cfd` 或 `sockfd`。
- 使用 [[linux网络编程/指令查询/命令卡/nc|nc]] 测试 TCP 服务：服务端运行后，可以用 `nc 127.0.0.1 9527` 连接服务端，手动输入数据观察回显。

## 知识点补充

- TCP 通信不是“服务端直接对监听 socket 读写”，而是对 `accept` 返回的新 socket 读写。
- 读到返回值 `0` 通常表示对端关闭连接。
- 如果只写单客户端案例，服务器可以只 `accept` 一次；后续高并发章节会把 `accept` 放进循环。
- 示例里也可能使用 `read/write` 操作 socket，因为 Linux socket 本质上也是文件描述符。

## 本节内容速览

- 服务器：`socket -> bind -> listen -> accept -> recv/read -> send/write`。
- 客户端：`socket -> connect -> send/write -> recv/read`。
- 测试：可以使用 `nc` 作为简单客户端连接服务端。

## 复习时要回答

- 一个最基本的 TCP 小写转大写案例包含哪些步骤？
- 为什么服务端要区分 `lfd` 和 `cfd`？
- `recv/read` 返回 `0` 表示什么？
- 如何用 `nc` 测试自己写的 TCP 服务器？

## 本节关键函数

- [[linux网络编程/函数笔记/Socket/socket|socket]]
- [[linux网络编程/函数笔记/Socket/accept|accept]]
- [[linux网络编程/函数笔记/Socket/connect|connect]]
- [[linux网络编程/函数笔记/Socket/send|send]]
- [[linux网络编程/函数笔记/Socket/recv|recv]]

## 本节关键概念

- [[linux网络编程/概念词条/TCP|TCP]]
- [[linux网络编程/概念词条/TCP通信流程|TCP通信流程]]
- [[linux网络编程/概念词条/已连接套接字|已连接套接字]]
- [[linux网络编程/概念词条/阻塞式IO|阻塞式IO]]

## 本节关键指令

- [[linux网络编程/指令查询/命令卡/nc|nc]]

## 关联模块

- [[linux网络编程/03 TCP通信与通信案例|03 TCP通信与通信案例]]
