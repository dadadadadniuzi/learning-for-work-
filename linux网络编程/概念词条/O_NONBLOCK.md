---
title: O_NONBLOCK
tags:
  - linux
  - 网络编程
  - 概念词条
  - 网络编程/IO多路复用
---
# O_NONBLOCK

## 它是什么

`O_NONBLOCK` 是一个文件状态标志，用来把文件描述符设置为非阻塞模式。

在网络编程里，它最常见的用法是配合 [[linux系统编程/函数笔记/文件IO/fcntl|fcntl]]，把 socket 设置成非阻塞。

## 常见写法

```c
int flags = fcntl(fd, F_GETFL, 0);
fcntl(fd, F_SETFL, flags | O_NONBLOCK);
```

## 怎么理解

设置 `O_NONBLOCK` 后：

- 如果当前操作可以立刻完成，就正常完成。
- 如果当前条件暂时不满足，就立即返回，而不是阻塞等待。

## 在网络编程中的典型影响

- `accept`：没有新连接时立刻返回。
- `read/recv`：没有数据时立刻返回。
- `send/write`：暂时不能写时立刻返回。

这时通常需要检查 [[linux网络编程/概念词条/EAGAIN与EWOULDBLOCK|EAGAIN / EWOULDBLOCK]]。

## 易错点

- `O_NONBLOCK` 通常不是单独直接覆盖写入，而是和原有标志按位或。
- 它是 fd 状态标志，不是一次性调用参数；如果只想让某一次 `recv` 非阻塞，也可以考虑 `MSG_DONTWAIT`。
- ET 模式下通常要配合 `O_NONBLOCK`。

## 相关函数

- [[linux系统编程/函数笔记/文件IO/fcntl|fcntl]]
- [[linux网络编程/函数笔记/Socket/recv|recv]]
- [[linux网络编程/函数笔记/Socket/send|send]]
- [[linux网络编程/函数笔记/Socket/accept|accept]]

## 相关概念

- [[linux网络编程/概念词条/非阻塞I O|非阻塞I/O]]
- [[linux网络编程/概念词条/EAGAIN与EWOULDBLOCK|EAGAIN与EWOULDBLOCK]]
- [[linux系统编程/概念词条/open标志位|open标志位]]
- [[linux系统编程/概念词条/fcntl命令|fcntl命令]]

## 相关课时

- [[linux网络编程/课时笔记/05 IO多路复用/04 非阻塞I O|04 非阻塞I/O]]
