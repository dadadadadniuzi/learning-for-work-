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
- [[linux网络编程/概念词条/TCP三次握手|TCP三次握手]]
- [[linux网络编程/概念词条/TCP四次挥手|TCP四次挥手]]
- [[linux网络编程/概念词条/TCP数据包格式|TCP数据包格式]]
- [[linux网络编程/概念词条/TCP滑动窗口|TCP滑动窗口]]
- [[linux网络编程/概念词条/TCP通信时序与代码对应关系|TCP通信时序与代码对应关系]]
- [[linux网络编程/概念词条/TCP状态转换图|TCP状态转换图]]
- 小写转大写服务器的处理流程
- 连接建立后的读写流程
- 使用 [[linux网络编程/指令查询/命令卡/nc|nc]] 测试 TCP 服务

## 本节学什么详解

- TCP 客户端与服务器基本案例：服务器先创建监听 socket，绑定 IP 和端口，进入监听状态，然后通过 `accept` 等待客户端；客户端创建 socket 后使用 `connect` 主动连接服务器。
- [[linux网络编程/概念词条/TCP三次握手|TCP三次握手]]：客户端 `connect` 和服务器 `accept` 背后会完成三次握手，确认双方收发能力并同步初始序号。
- [[linux网络编程/概念词条/TCP四次挥手|TCP四次挥手]]：连接关闭时，双方通过 FIN 和 ACK 释放连接；代码中常见表现是 [[linux网络编程/函数笔记/Socket/recv|recv]] / [[linux网络编程/函数笔记/Socket/read|read]] 返回 `0`。
- [[linux网络编程/概念词条/TCP数据包格式|TCP数据包格式]]：TCP 首部里的源端口、目的端口、序号、确认号、标志位和窗口大小，支撑连接管理和可靠传输。
- [[linux网络编程/概念词条/TCP滑动窗口|TCP滑动窗口]]：TCP 不需要每发送一小段数据就停下来等确认，而是在窗口范围内连续发送，并根据 ACK 向前滑动。
- [[linux网络编程/概念词条/TCP通信时序与代码对应关系|TCP通信时序与代码对应关系]]：把客户端 `socket/connect/write/read/close` 和服务器 `socket/bind/listen/accept/read/write/close` 对应到 TCP 状态变化与网络报文。
- [[linux网络编程/概念词条/TCP状态转换图|TCP状态转换图]]：从 TCP 状态机角度观察 `CLOSED`、`LISTEN`、`SYN_SENT`、`ESTABLISHED`、`FIN_WAIT_*`、`TIME_WAIT` 的切换。
- 小写转大写服务器的处理流程：服务器收到客户端发来的字符串后，对每个字符调用转换逻辑，例如 `toupper`，再把处理后的数据写回客户端。
- 连接建立后的读写流程：连接建立后，服务器和客户端都围绕已连接 socket 读写数据。服务器端通常使用 `cfd`，客户端通常使用 `cfd` 或 `sockfd`。
- 使用 [[linux网络编程/指令查询/命令卡/nc|nc]] 测试 TCP 服务：服务端运行后，可以用 `nc 127.0.0.1 9527` 连接服务端，手动输入数据观察回显。

## 知识点补充

- TCP 通信不是“服务端直接对监听 socket 读写”，而是对 `accept` 返回的新 socket 读写。
- `connect` 成功通常意味着 [[linux网络编程/概念词条/TCP三次握手|TCP三次握手]] 已经完成。
- `send/write` 交给内核的数据，会被封装进 [[linux网络编程/概念词条/TCP数据包格式|TCP报文段]]，并由 [[linux网络编程/概念词条/TCP滑动窗口|TCP滑动窗口]] 控制发送节奏。
- 读到返回值 `0` 通常表示对端关闭连接。
- 如果只写单客户端案例，服务器可以只 `accept` 一次；后续高并发章节会把 `accept` 放进循环。
- 示例里也可能使用 [[linux网络编程/函数笔记/Socket/read|read]] / `write` 操作 socket，因为 Linux socket 本质上也是文件描述符。

## 本节内容速览

- 服务器：`socket -> bind -> listen -> accept -> recv/read -> send/write`，其中 [[linux网络编程/函数笔记/Socket/read|read]] 在基础 TCP 读取场景中很常见。
- 客户端：`socket -> connect -> send/write -> recv/read`。
- 测试：可以使用 `nc` 作为简单客户端连接服务端。

## 复习时要回答

- 一个最基本的 TCP 小写转大写案例包含哪些步骤？
- 为什么服务端要区分 `lfd` 和 `cfd`？
- [[linux网络编程/函数笔记/Socket/recv|recv]] / [[linux网络编程/函数笔记/Socket/read|read]] 返回 `0` 表示什么？
- 如何用 `nc` 测试自己写的 TCP 服务器？

## 本节关键函数

- [[linux网络编程/函数笔记/Socket/socket|socket]]
- [[linux网络编程/函数笔记/Socket/accept|accept]]
- [[linux网络编程/函数笔记/Socket/connect|connect]]
- [[linux网络编程/函数笔记/Socket/send|send]]
- [[linux网络编程/函数笔记/Socket/recv|recv]]
- [[linux网络编程/函数笔记/Socket/read|read]]

## 本节关键概念

- [[linux网络编程/概念词条/TCP|TCP]]
- [[linux网络编程/概念词条/TCP通信流程|TCP通信流程]]
- [[linux网络编程/概念词条/TCP三次握手|TCP三次握手]]
- [[linux网络编程/概念词条/TCP四次挥手|TCP四次挥手]]
- [[linux网络编程/概念词条/TCP数据包格式|TCP数据包格式]]
- [[linux网络编程/概念词条/TCP滑动窗口|TCP滑动窗口]]
- [[linux网络编程/概念词条/TCP通信时序与代码对应关系|TCP通信时序与代码对应关系]]
- [[linux网络编程/概念词条/TCP状态转换图|TCP状态转换图]]
- [[linux网络编程/概念词条/已连接套接字|已连接套接字]]
- [[linux网络编程/概念词条/阻塞式IO|阻塞式IO]]

## 本节关键指令

- [[linux网络编程/指令查询/命令卡/nc|nc]]

## 关联模块

- [[linux网络编程/03 TCP通信与通信案例|03 TCP通信与通信案例]]
