---
title: shutdown
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/Socket
---
# shutdown

> [!info] 功能
> 关闭一个已连接 socket 的读方向、写方向或读写两个方向，常用于实现 [[linux网络编程/概念词条/TCP半关闭|TCP半关闭]]。

## 函数原型

- `int shutdown(int sockfd, int how);`

## 依赖头文件

- `#include <sys/socket.h>`

## 输入参数

- `sockfd`：已经连接的 socket 文件描述符，通常来自 [[linux网络编程/函数笔记/Socket/socket|socket]]、[[linux网络编程/函数笔记/Socket/accept|accept]] 或 [[linux网络编程/函数笔记/Socket/connect|connect]] 后的通信 fd。
- `how`：关闭方向，决定关闭读、写还是读写两个方向。

## how 参数取值

- `SHUT_RD`：关闭读方向。本端不再接收数据。
- `SHUT_WR`：关闭写方向。本端不能再发送数据，会向对端发送 FIN；本端仍然可以继续接收数据。
- `SHUT_RDWR`：同时关闭读方向和写方向。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回 `0`。
- 失败返回 `-1`，并设置 `errno`。

## 知识点补充

- `shutdown(fd, SHUT_WR)` 是学习 [[linux网络编程/概念词条/TCP半关闭|TCP半关闭]] 的重点。
- 对端读完剩余数据后，`read/recv` 通常返回 `0`，表示本端写方向已经关闭。
- `shutdown` 作用于 socket 连接方向；`close` 作用于文件描述符引用。一个 socket 被多个 fd 引用时，`shutdown` 的语义更直接。

## 常见用法

```c
shutdown(fd, SHUT_WR);
```

表示本端不再发送数据，但仍然保留接收能力。

## 易错点

- `SHUT_WR` 之后不要再调用 [[linux网络编程/函数笔记/Socket/send|send]] 或 `write` 发送数据。
- `shutdown` 不等于释放 fd，最终通常仍需要 `close(fd)`。
- `read/recv` 返回 `0` 只说明对端关闭了写方向，不代表本端 fd 已经自动释放。

## 相关概念

- [[linux网络编程/概念词条/TCP半关闭|TCP半关闭]]
- [[linux网络编程/概念词条/TCP四次挥手|TCP四次挥手]]
- [[linux网络编程/概念词条/TCP状态转换图|TCP状态转换图]]

## 相关课时

- [[linux网络编程/课时笔记/03 TCP通信与通信案例/02 客户端与服务器通信流程|02 客户端与服务器通信流程]]

## 相关模块

- [[linux网络编程/03 TCP通信与通信案例|03 TCP通信与通信案例]]
