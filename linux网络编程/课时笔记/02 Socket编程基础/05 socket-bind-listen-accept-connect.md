---
title: socket-bind-listen-accept-connect
tags:
  - linux
  - 网络编程
  - 课时笔记
  - 网络编程/Socket编程基础
---
# socket-bind-listen-accept-connect

## 本节学什么

- `socket`
- `bind`
- `listen`
- `accept`
- `connect`
- 服务器端和客户端的基础调用链

## 本节学什么详解

- `socket`：创建套接字，得到一个文件描述符，后续所有网络操作都围绕这个描述符展开。
- `bind`：把服务器 socket 绑定到本机 IP 和端口，让客户端知道该连接到哪里。
- `listen`：把主动套接字转换成监听套接字，准备接收客户端连接请求。设置同时与服务器建立连接的上限数。（同时进行3次握手的客户端数量）
- `accept`：阻塞等待或取出一个已完成连接，**返回新的已连接套接字**。
- `connect`：客户端主动向服务器地址发起连接。
- 服务器端调用链：`socket -> bind -> listen -> accept -> read/write`。
- 客户端调用链：`socket -> connect -> read/write`。

## 知识点补充

- **`socket` 返回的监听描述符和 `accept` 返回的通信描述符不是同一个**。
- `bind` 绑定的是服务器自己的地址，不是客户端地址。
- `listen` 的 `backlog` 表示等待连接队列相关的上限，不是“最多只能服务这么多个客户端”的完整含义。
- `accept` 的 `addr` 和 `addrlen` 是输出参数，用来带出客户端地址信息。
- `connect` 需要传入服务器地址结构。

## 本节内容速览

- 服务器：创建、绑定、监听、接收连接。
- 客户端：创建、主动连接。
- 连接建立后，双方使用通信 socket 读写数据。

## 复习时要回答

- 为什么服务器端需要 `bind`，客户端通常不显式 `bind`？
- `listen` 和 `accept` 分别做什么？
- `accept` 返回的新文件描述符和 `socket` 返回的监听描述符有什么区别？

## 本节关键函数

- [[linux网络编程/函数笔记/Socket/socket|socket]]
- [[linux网络编程/函数笔记/Socket/bind|bind]]
- [[linux网络编程/函数笔记/Socket/listen|listen]]
- [[linux网络编程/函数笔记/Socket/accept|accept]]
- [[linux网络编程/函数笔记/Socket/connect|connect]]

## 本节关键概念

- [[linux网络编程/概念词条/套接字|套接字]]
- [[linux网络编程/概念词条/监听套接字|监听套接字]]
- [[linux网络编程/概念词条/已连接套接字|已连接套接字]]
- [[linux网络编程/概念词条/地址族|地址族]]
- [[linux网络编程/概念词条/套接字类型|套接字类型]]

## 关联模块

- [[linux网络编程/02 Socket编程基础|02 Socket编程基础]]
