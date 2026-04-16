---
title: Socket编程基础
tags:
  - linux
  - 网络编程
  - 模块复习
  - 网络编程/Socket编程基础
---
# Socket编程基础

## 本章目标

- 理解[[linux网络编程/概念词条/套接字|套接字]]是应用程序访问网络协议栈的入口。
- 掌握[[linux网络编程/概念词条/网络字节序|网络字节序]]和[[linux网络编程/概念词条/大端与小端|大端与小端]]，知道为什么端口和 IP 地址要转换。
- 学会使用 `inet_pton` / `inet_ntop` 在字符串 IP 和网络地址之间转换。
- 理解 [[linux网络编程/概念词条/sockaddr|sockaddr]]、[[linux网络编程/概念词条/sockaddr_in|sockaddr_in]]、[[linux网络编程/概念词条/socklen_t|socklen_t]] 在 socket API 中的作用。
- 建立服务器端 `socket -> bind -> listen -> accept` 和客户端 `socket -> connect` 的基本流程。

## 核心函数

- [[linux网络编程/函数笔记/网络字节序/htonl|htonl]]
- [[linux网络编程/函数笔记/网络字节序/htons|htons]]
- [[linux网络编程/函数笔记/网络字节序/ntohl|ntohl]]
- [[linux网络编程/函数笔记/网络字节序/ntohs|ntohs]]
- [[linux网络编程/函数笔记/地址转换/inet_pton|inet_pton]]
- [[linux网络编程/函数笔记/地址转换/inet_ntop|inet_ntop]]
- [[linux网络编程/函数笔记/Socket/socket|socket]]
- [[linux网络编程/函数笔记/Socket/bind|bind]]
- [[linux网络编程/函数笔记/Socket/listen|listen]]
- [[linux网络编程/函数笔记/Socket/accept|accept]]
- [[linux网络编程/函数笔记/Socket/connect|connect]]

## 本模块课时

- [[linux网络编程/课时笔记/02 Socket编程基础/01 套接字与socket模型|01 套接字与socket模型]]
- [[linux网络编程/课时笔记/02 Socket编程基础/02 网络字节序|02 网络字节序]]
- [[linux网络编程/课时笔记/02 Socket编程基础/03 IP地址转换函数|03 IP地址转换函数]]
- [[linux网络编程/课时笔记/02 Socket编程基础/04 sockaddr与sockaddr_in结构|04 sockaddr与sockaddr_in结构]]
- [[linux网络编程/课时笔记/02 Socket编程基础/05 socket-bind-listen-accept-connect|05 socket-bind-listen-accept-connect]]

## 本模块概念

- [[linux网络编程/概念词条/套接字|套接字]]
- [[linux网络编程/概念词条/地址族|地址族]]
- [[linux网络编程/概念词条/套接字类型|套接字类型]]
- [[linux网络编程/概念词条/网络字节序|网络字节序]]
- [[linux网络编程/概念词条/IP地址|IP地址]]
- [[linux网络编程/概念词条/端口|端口]]
- [[linux网络编程/概念词条/sockaddr|sockaddr]]
- [[linux网络编程/概念词条/sockaddr_in|sockaddr_in]]
- [[linux网络编程/概念词条/in_addr|in_addr]]
- [[linux网络编程/概念词条/监听套接字|监听套接字]]
- [[linux网络编程/概念词条/已连接套接字|已连接套接字]]

## 细节补充

- 套接字在 Linux 中表现为文件描述符，所以后续通信可以使用 `read/write` 或 `send/recv` 等接口。
- 服务器端调用 `socket` 得到的描述符通常先作为[[linux网络编程/概念词条/监听套接字|监听套接字]]使用；`accept` 返回的新描述符才是和某个客户端通信的[[linux网络编程/概念词条/已连接套接字|已连接套接字]]。
- 网络中多字节整数使用大端字节序，端口号通常用 `htons` 转换，IPv4 地址常用 `htonl` 或 `inet_pton` 得到网络字节序表示。
- `sockaddr_in` 是 IPv4 地址结构，很多 socket API 参数写成通用的 `struct sockaddr *`，所以实际调用时常见 `(struct sockaddr *)&addr` 这种强制转换。

## 复习路线

- 先理解套接字是什么，以及为什么一条 TCP 连接两端各有一个 socket。
- 再掌握字节序转换和 IP 地址转换。
- 最后把 `socket/bind/listen/accept/connect` 串成服务器端和客户端两条调用链。
