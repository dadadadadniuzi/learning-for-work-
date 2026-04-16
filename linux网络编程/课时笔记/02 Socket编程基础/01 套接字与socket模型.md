---
title: 套接字与socket模型
tags:
  - linux
  - 网络编程
  - 课时笔记
  - 网络编程/Socket编程基础
---
# 套接字与socket模型

## 本节学什么

- [[linux网络编程/概念词条/套接字|套接字]]
- socket 在 Linux 中和文件描述符的关系
- TCP 通信中[[linux网络编程/概念词条/监听套接字|监听套接字]]与[[linux网络编程/概念词条/已连接套接字|已连接套接字]]
- `socket` 模型的基本创建流程

## 本节学什么详解

- [[linux网络编程/概念词条/套接字|套接字]]：套接字是应用程序和内核网络协议栈之间的接口。应用程序不直接操作网卡，而是通过 socket 把数据交给内核，由内核完成协议封装、发送和接收。
- socket 与文件描述符：在 Linux 中，`socket` 调用成功后返回一个整数文件描述符。这个描述符可以被 `read/write/close` 等文件 I/O 思路管理，也可以使用网络专用的 `send/recv`。
- [[linux网络编程/概念词条/监听套接字|监听套接字]]与[[linux网络编程/概念词条/已连接套接字|已连接套接字]]：服务器 `socket` 创建出的描述符经过 `bind/listen` 后用于监听连接；每当 `accept` 成功，会返回一个新的描述符用于和具体客户端通信。
- socket 模型创建流程：服务器侧通常是 `socket -> bind -> listen -> accept -> read/write`，客户端侧通常是 `socket -> connect -> read/write`。

## 知识点补充

- “套接字成对出现”指通信双方各自拥有一个 socket 端点。
- 服务器端常见变量名 `lfd` 表示 listen fd，`cfd` 表示 connected/client fd。
- 一个监听 socket 可以不断 `accept` 多个客户端连接，每个成功连接都会生成一个新的通信 socket。

## 本节内容速览

- socket 是网络通信入口。
- socket 在 Linux 中也是文件描述符。
- 监听套接字负责等连接，已连接套接字负责实际通信。

## 复习时要回答

- socket 为什么可以理解成“网络通信的文件描述符”？
- 监听套接字和已连接套接字有什么区别？
- 服务器端和客户端创建连接的流程分别是什么？

## 本节关键函数

- [[linux网络编程/函数笔记/Socket/socket|socket]]
- [[linux网络编程/函数笔记/Socket/accept|accept]]

## 本节关键概念

- [[linux网络编程/概念词条/套接字|套接字]]
- [[linux网络编程/概念词条/监听套接字|监听套接字]]
- [[linux网络编程/概念词条/已连接套接字|已连接套接字]]

## 关联模块

- [[linux网络编程/02 Socket编程基础|02 Socket编程基础]]
