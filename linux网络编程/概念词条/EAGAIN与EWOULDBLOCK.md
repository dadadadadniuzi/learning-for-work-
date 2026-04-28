---
title: EAGAIN与EWOULDBLOCK
tags:
  - linux
  - 网络编程
  - 概念词条
  - 网络编程/IO多路复用
---
# EAGAIN 与 EWOULDBLOCK

## 它们是什么

`EAGAIN` 和 `EWOULDBLOCK` 是常见错误码。

在网络编程的非阻塞 I/O 场景中，它们通常表示：

当前操作现在做不了，但不是致命错误，可以稍后再试。

## 什么时候常见

- 非阻塞 `accept`：当前没有新连接。
- 非阻塞 `read/recv`：当前没有数据可读。
- 非阻塞 `send/write`：当前暂时不能继续写。

## 怎么理解

如果 fd 是阻塞模式：

- 条件不满足时，函数会等。

如果 fd 是非阻塞模式：

- 条件不满足时，函数立刻返回 `-1`。
- 然后把 `errno` 设为 `EAGAIN` 或 `EWOULDBLOCK`。

所以这两个错误码常常意味着：

```text
不是彻底失败
只是现在还不能继续
```

## 常见写法

```c
ssize_t n = recv(fd, buf, sizeof(buf), 0);
if (n == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // 当前没有数据，稍后再试
    } else {
        // 真正错误
    }
}
```

## 和 ET 模式的关系

在 [[linux网络编程/概念词条/水平触发与边沿触发|边沿触发 ET]] 下，常见套路是：

```text
收到“可读”通知
  ↓
循环 recv / read
  ↓
直到 errno == EAGAIN 或 EWOULDBLOCK
```

这时就表示当前缓冲区已经被你读空，可以先停下来，等下一次真正有新数据到来。

## 两者是否相同

在很多系统和很多网络编程场景里，`EAGAIN` 与 `EWOULDBLOCK` 常常取值相同，或者至少可以用同样方式处理。

学习阶段最实用的记法是：

- 判断时同时检查 `EAGAIN || EWOULDBLOCK`。
- 把它们都理解为“现在先别继续，稍后再试”。

## 易错点

- 不要把 `EAGAIN` / `EWOULDBLOCK` 当成连接断开。
- 返回 `0` 和返回 `-1 + EAGAIN` 含义不同：
  - 返回 `0`：TCP 中通常表示对端关闭连接。
  - 返回 `-1` 且 `errno == EAGAIN/EWOULDBLOCK`：只是当前暂时没有数据。
- 如果是其他 `errno`，就要按真正错误处理。

## 相关函数

- [[linux网络编程/函数笔记/Socket/accept|accept]]
- [[linux网络编程/函数笔记/Socket/read|read]]
- [[linux网络编程/函数笔记/Socket/recv|recv]]
- [[linux网络编程/函数笔记/Socket/send|send]]

## 相关概念

- [[linux网络编程/概念词条/非阻塞I O|非阻塞I/O]]
- [[linux网络编程/概念词条/O_NONBLOCK|O_NONBLOCK]]
- [[linux网络编程/概念词条/水平触发与边沿触发|水平触发与边沿触发]]
- [[linux系统编程/概念词条/errno|errno]]

## 相关课时

- [[linux网络编程/课时笔记/05 IO多路复用/04 非阻塞I O|04 非阻塞I/O]]
- [[linux网络编程/课时笔记/05 IO多路复用/03 epoll|03 epoll]]
