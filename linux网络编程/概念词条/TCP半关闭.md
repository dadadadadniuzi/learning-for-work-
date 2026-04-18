---
title: TCP半关闭
tags:
  - linux
  - 网络编程
  - 概念词条
  - 网络编程/TCP通信
---
# TCP半关闭

TCP 半关闭指只关闭 TCP 连接的一个方向，而不是立刻把读写两个方向都关闭。它利用了 [[linux网络编程/概念词条/TCP|TCP]] 全双工通信的特点：一条 TCP 连接中，发送方向和接收方向可以分别结束。

## 它解决什么问题

- 一端想告诉对方“我已经没有数据要发了”，但仍然想继续接收对方的数据。
- 应用层协议需要明确表达“请求发送完毕”，同时等待服务器继续返回结果。
- 避免直接 `close` 导致本端读写都不可用。

## 怎么实现

半关闭通常使用 [[linux网络编程/函数笔记/Socket/shutdown|shutdown]]。

```c
shutdown(fd, SHUT_WR);
```

这表示关闭本端写方向：本端不能再发送数据，但仍然可以继续 `read/recv` 接收对端数据。

## shutdown 的三种关闭方式

- `SHUT_RD`：关闭读方向，本端不再接收数据。
- `SHUT_WR`：关闭写方向，本端发送 FIN，告诉对端“我不再发送数据”，但还可以继续读。
- `SHUT_RDWR`：读写方向都关闭。

## 和 close 的区别

| 操作 | 效果 |
|---|---|
| `close(fd)` | 关闭文件描述符。通常表示应用层不再使用这个 socket；若引用计数归零，内核会关闭连接。 |
| `shutdown(fd, SHUT_WR)` | 只关闭写方向，触发 FIN，但本端仍可继续读。 |
| `shutdown(fd, SHUT_RD)` | 只关闭读方向。 |
| `shutdown(fd, SHUT_RDWR)` | 关闭读写方向，语义接近完全关闭通信。 |

## 和四次挥手的关系

半关闭会触发 [[linux网络编程/概念词条/TCP四次挥手|TCP四次挥手]] 的一部分。

当一端执行 `shutdown(fd, SHUT_WR)`：

- 本端发送 `FIN`，表示写方向结束。
- 对端继续 `read/recv` 时，读完剩余数据后会读到 `0`。
- 对端仍然可以继续写数据返回给本端。
- 本端仍然可以继续 `read/recv` 接收对端后续数据。

## 典型流程

```text
客户端 write 发送请求
客户端 shutdown(fd, SHUT_WR)
服务器 read 直到返回 0，知道客户端请求发送完毕
服务器继续 write 返回响应
客户端继续 read 接收响应
双方最终 close
```

## 易错点

- `read/recv` 返回 `0` 表示对端关闭了写方向，不一定表示本端不能继续写。
- `shutdown(fd, SHUT_WR)` 后不能再对该 fd 发送数据，否则会失败。
- `close` 是关闭描述符；`shutdown` 是关闭连接方向。二者语义不同。
- 半关闭只适用于面向连接的通信，典型场景是 TCP。

## 相关概念

- [[linux网络编程/概念词条/TCP|TCP]]
- [[linux网络编程/概念词条/TCP四次挥手|TCP四次挥手]]
- [[linux网络编程/概念词条/TCP状态转换图|TCP状态转换图]]
- [[linux网络编程/概念词条/TCP通信流程|TCP通信流程]]

## 相关函数

- [[linux网络编程/函数笔记/Socket/shutdown|shutdown]]
- [[linux网络编程/函数笔记/Socket/send|send]]
- [[linux网络编程/函数笔记/Socket/recv|recv]]

## 相关课时

- [[linux网络编程/课时笔记/03 TCP通信与通信案例/02 客户端与服务器通信流程|02 客户端与服务器通信流程]]

## 相关模块

- [[linux网络编程/03 TCP通信与通信案例|03 TCP通信与通信案例]]
