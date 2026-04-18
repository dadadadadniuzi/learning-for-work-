---
title: TCP通信与通信案例
tags:
  - linux
  - 网络编程
  - 模块复习
  - 网络编程/TCP通信
---
# TCP通信与通信案例

## 本章目标

- 串起 TCP 服务器端和客户端的基本通信流程。
- 理解连接建立后，双方如何通过[[linux网络编程/概念词条/已连接套接字|已连接套接字]]读写数据。
- 通过“小写转大写”案例理解[[linux网络编程/概念词条/阻塞式IO|阻塞式 IO]] 下的 TCP 通信。
- 学会使用 [[linux网络编程/指令查询/命令卡/nc|nc]] 快速测试 TCP 服务端。
- 为后续[[linux网络编程/04 高并发服务器|高并发服务器]]和[[linux网络编程/05 IO多路复用|IO 多路复用]]做准备。

## 核心函数

- [[linux网络编程/函数笔记/Socket/socket|socket]]
- [[linux网络编程/函数笔记/Socket/connect|connect]]
- [[linux网络编程/函数笔记/Socket/accept|accept]]
- [[bind]]
- [[linux网络编程/函数笔记/Socket/send|send]]
- [[linux网络编程/函数笔记/Socket/recv|recv]]
- [[linux网络编程/函数笔记/Socket/read|read]]
- [[shutdown]]
- [[getsockopt]]
- [[setsockopt]]

## 本模块课时

- [[linux网络编程/课时笔记/03 TCP通信与通信案例/01 TCP通信基础案例|01 TCP通信基础案例]]
- [[linux网络编程/课时笔记/03 TCP通信与通信案例/02 客户端与服务器通信流程|02 客户端与服务器通信流程]]

## 本模块概念

- [[linux网络编程/概念词条/TCP|TCP]]
- [[linux网络编程/概念词条/TCP通信流程|TCP通信流程]]
- [[linux网络编程/概念词条/TCP三次握手|TCP三次握手]]
- [[linux网络编程/概念词条/TCP四次挥手|TCP四次挥手]]
- [[linux网络编程/概念词条/TCP滑动窗口|TCP滑动窗口]]
- [[linux网络编程/概念词条/TCP数据包格式|TCP数据包格式]]
- [[linux网络编程/概念词条/TCP通信时序与代码对应关系|TCP通信时序与代码对应关系]]
- [[linux网络编程/概念词条/TCP状态转换图|TCP状态转换图]]
- [[linux网络编程/概念词条/TCP半关闭|TCP半关闭]]
- [[linux网络编程/概念词条/阻塞式IO|阻塞式IO]]
- [[linux网络编程/概念词条/监听套接字|监听套接字]]
- [[linux网络编程/概念词条/已连接套接字|已连接套接字]]
- [[linux网络编程/概念词条/send-recv标志|send-recv标志]]

## 本模块指令

- [[linux网络编程/指令查询/命令卡/nc|nc]]

## 细节补充

- TCP 通信代码通常分成“建立连接”和“数据读写”两段。服务器端负责监听和接收连接，客户端负责主动连接。
- 连接建立阶段由内核完成 [[linux网络编程/概念词条/TCP三次握手|TCP三次握手]]，应用层看到的是 `connect` 成功或 `accept` 返回新 fd。
- 连接关闭阶段通常对应 [[linux网络编程/概念词条/TCP四次挥手|TCP四次挥手]]，应用层常见现象是 [[linux网络编程/函数笔记/Socket/recv|recv]] / [[linux网络编程/函数笔记/Socket/read|read]] 返回 `0`。
- 如果只关闭发送方向但继续接收数据，就是 [[linux网络编程/概念词条/TCP半关闭|TCP半关闭]]，常用 [[linux网络编程/函数笔记/Socket/shutdown|shutdown]] 实现。
- 数据传输阶段依赖 [[linux网络编程/概念词条/TCP数据包格式|TCP数据包格式]] 中的序号、确认号、窗口字段，以及 [[linux网络编程/概念词条/TCP滑动窗口|TCP滑动窗口]] 机制。
- 用 [[linux网络编程/概念词条/TCP通信时序与代码对应关系|TCP通信时序与代码对应关系]] 可以把客户端代码、服务器代码和 TCP 状态变化逐行对上。
- 用 [[linux网络编程/概念词条/TCP状态转换图|TCP状态转换图]] 可以从完整状态机角度理解主动打开、被动打开、主动关闭、被动关闭和同时关闭。
- 服务器端 `accept` 返回的 `cfd` 才用于和客户端通信，原来的 `lfd` 继续负责监听新连接。
- 在阻塞模式下，如果没有客户端连接，`accept` 会等待；如果没有数据到达，[[linux网络编程/函数笔记/Socket/recv|recv]] / [[linux网络编程/函数笔记/Socket/read|read]] 会等待。
- TCP 是字节流协议，不能假设一次 `send/write` 一定对应对端一次 [[linux网络编程/函数笔记/Socket/recv|recv]] / [[linux网络编程/函数笔记/Socket/read|read]]。

## 复习路线

- 先背出服务器端和客户端各自的调用链。
- 再把调用链和 [[linux网络编程/概念词条/TCP三次握手|TCP三次握手]]、[[linux网络编程/概念词条/TCP四次挥手|TCP四次挥手]] 对应起来。
- 继续理解 [[linux网络编程/概念词条/TCP数据包格式|TCP数据包格式]] 和 [[linux网络编程/概念词条/TCP滑动窗口|TCP滑动窗口]] 如何支撑可靠传输。
- 再理解 `lfd`、`cfd`、客户端 socket 分别代表什么。
- 最后用 `nc` 或自写客户端测试服务器程序，观察连接、收发、关闭三个阶段。
