---
title: TCP
tags:
  - linux
  - 网络编程
  - 概念词条
---
# TCP

## 它是什么

- TCP 是传输层协议，全称 Transmission Control Protocol。
- 它提供面向连接、可靠、有序、基于字节流的通信能力。

## 怎么理解

- 面向连接：通信前通常需要先建立连接。
- 可靠：TCP 会通过确认、重传、序号等机制尽量保证数据可靠到达。
- 有序：接收方看到的数据顺序和发送方写入顺序一致。
- 字节流：TCP 不保留应用层每次写入的消息边界，程序需要自己处理粘包、拆包等问题。

## 在 socket 中的典型位置

- `socket(AF_INET, SOCK_STREAM, 0)` 通常创建 TCP 套接字。
- 服务器端常见流程是 `socket -> bind -> listen -> accept -> read/write`。
- 客户端常见流程是 `socket -> connect -> read/write`。

## 和 UDP 的区别

- TCP 更适合需要可靠传输、有序传输的场景。
- [[linux网络编程/概念词条/UDP|UDP]] 更轻量，但不保证可靠到达和顺序。

## 易错点

- TCP 是字节流协议，不是消息包协议。
- `write` 一次不等于对端 `read` 一次。
- TCP 的可靠性是协议层提供的，但应用层仍然要正确处理返回值、连接关闭和异常。

## 常见出现位置

- [[linux网络编程/01 网络基础|01 网络基础]]
- [[linux网络编程/03 TCP通信与通信案例|03 TCP通信与通信案例]]
- [[linux网络编程/函数笔记/Socket/socket|socket]]
