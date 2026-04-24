---
title: UDP通信流程
tags:
  - linux
  - 网络编程
  - 概念词条
  - 网络编程/UDP
---
# UDP通信流程

## 它是什么

UDP 通信流程是指使用 [[linux网络编程/概念词条/UDP|UDP]] socket 完成数据报收发时，服务器端和客户端各自的函数调用顺序。

UDP 的关键特点是无连接、面向数据报。因此它没有 TCP 的 [[linux网络编程/概念词条/TCP三次握手|三次握手]]、[[linux网络编程/函数笔记/Socket/listen|listen]]、[[linux网络编程/函数笔记/Socket/accept|accept]] 这一套流程。

## 服务器端调用链

```text
socket
  ↓
bind
  ↓
recvfrom
  ↓
sendto
```

服务器端通常需要 [[linux网络编程/函数笔记/Socket/bind|bind]] 到固定端口，这样客户端才能知道目标地址。

## 客户端调用链

```text
socket
  ↓
sendto
  ↓
recvfrom
```

客户端通常不需要主动 [[linux网络编程/函数笔记/Socket/bind|bind]]，内核会自动选择临时端口。客户端也通常不需要 [[linux网络编程/函数笔记/Socket/connect|connect]]，因为 [[linux网络编程/函数笔记/Socket/sendto|sendto]] 每次发送时都能指定目标地址。

## 地址信息怎么传递

UDP 通信里地址很重要：

- 客户端发送时，[[linux网络编程/函数笔记/Socket/sendto|sendto]] 参数里要带服务器地址。
- 服务器接收时，[[linux网络编程/函数笔记/Socket/recvfrom|recvfrom]] 可以把客户端地址写出来。
- 服务器回复时，把这个客户端地址交给 [[linux网络编程/函数笔记/Socket/sendto|sendto]]。

这也是 UDP 和 TCP 代码很不一样的地方：TCP 的 [[linux网络编程/函数笔记/Socket/accept|accept]] 会返回一个已连接 socket；UDP 没有已连接 socket，通常通过每次收发时的地址参数区分对端。

## 和 TCP 通信流程对比

| 阶段        | TCP                                           | UDP                              |
| --------- | --------------------------------------------- | -------------------------------- |
| 创建 socket | `socket(AF_INET, SOCK_STREAM, 0)`             | `socket(AF_INET, SOCK_DGRAM, 0)` |
| 服务器绑定端口   | 需要                                            | 通常需要                             |
| 监听连接      | [[listen]]                                    | 不需要                              |
| 接收连接      | [[accept]]                                    | 不需要                              |
| 客户端连接     | [[connect]]t                                  | 通常不需要                            |
| 发送数据      | [[send]]/`write`                              | [[sendto]]                       |
| 接收数据      | [[recv]]/[[linux网络编程/函数笔记/Socket/read\|read]] | [[recvfrom]]                     |

## 需要记住的核心点

- UDP 没有连接建立阶段。
- UDP 一次发送就是一个数据报。
- UDP 服务器只有一个 socket 也可以和多个客户端通信。
- UDP 服务器通过 [[linux网络编程/函数笔记/Socket/recvfrom|recvfrom]] 得到“谁发来的”。
- UDP 服务器通过 [[linux网络编程/函数笔记/Socket/sendto|sendto]] 指定“回复给谁”。

## 易错点

- 不要给 UDP 服务器写 [[linux网络编程/函数笔记/Socket/listen|listen]] / [[linux网络编程/函数笔记/Socket/accept|accept]]。
- 不要把 UDP 当成 TCP 字节流，UDP 保留数据报边界。
- 不要忘记初始化 `socklen_t addrlen`，否则 [[linux网络编程/函数笔记/Socket/recvfrom|recvfrom]] 写回地址时可能出错。
- UDP 不保证可靠传输，应用层如果需要确认、重传、编号，要自己设计协议。

## 相关函数

- [[linux网络编程/函数笔记/Socket/socket|socket]]
- [[linux网络编程/函数笔记/Socket/bind|bind]]
- [[linux网络编程/函数笔记/Socket/sendto|sendto]]
- [[linux网络编程/函数笔记/Socket/recvfrom|recvfrom]]

## 相关概念

- [[linux网络编程/概念词条/UDP|UDP]]
- [[linux网络编程/概念词条/UDP数据报格式|UDP数据报格式]]
- [[linux网络编程/概念词条/套接字类型|套接字类型]]
- [[linux网络编程/概念词条/sockaddr_in|sockaddr_in]]

## 相关课时

- [[linux网络编程/课时笔记/03 TCP通信与通信案例/03 UDP通信案例|03 UDP通信案例]]

## 相关模块

- [[linux网络编程/03 TCP通信与通信案例|03 TCP通信与通信案例]]
