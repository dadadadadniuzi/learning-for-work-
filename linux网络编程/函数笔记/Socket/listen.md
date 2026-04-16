---
title: listen
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/Socket
---
# listen

> [!info] 功能
> 将已绑定的 TCP socket 设置为监听状态，准备接收客户端连接。
> 设置同时与服务器建立连接的上限数。（同时进行3次握手的客户端数量）

## 函数原型

- `int listen(int sockfd, int backlog);`

## 依赖头文件

- `#include <sys/types.h>`
- `#include <sys/socket.h>`

## 输入参数

- `sockfd`：由 [[linux网络编程/函数笔记/Socket/socket|socket]] 创建、并通常已经经过 [[linux网络编程/函数笔记/Socket/bind|bind]] 绑定的 TCP 套接字。
- `backlog`：等待连接队列相关的上限提示值。它影响内核为该监听 socket 维护的连接队列大小，实际含义会受系统内核参数影响。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回 `0`。
- 失败返回 `-1`，并设置 `errno`。

## 知识点补充

- `listen` 之后，该 socket 成为[[linux网络编程/概念词条/监听套接字|监听套接字]]。
- `listen` 本身不返回客户端连接；真正取出连接的是 [[linux网络编程/函数笔记/Socket/accept|accept]]。

## 常见用法

```c
listen(lfd, 128);
```

## 易错点

- `listen` 只用于面向连接的套接字，例如 TCP。
- `backlog` **不是服务器最多能服务的客户端总数**。

## 相关概念

- [[linux网络编程/概念词条/监听套接字|监听套接字]]
- [[linux网络编程/概念词条/TCP|TCP]]

## 相关课时

- [[linux网络编程/课时笔记/02 Socket编程基础/05 socket-bind-listen-accept-connect|05 socket-bind-listen-accept-connect]]

## 相关模块

- [[linux网络编程/02 Socket编程基础|02 Socket编程基础]]
