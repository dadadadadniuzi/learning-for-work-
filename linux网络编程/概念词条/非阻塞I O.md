---
title: 非阻塞I O
tags:
  - linux
  - 网络编程
  - 概念词条
  - 网络编程/IO多路复用
---
# 非阻塞I/O

## 它是什么

非阻塞 I/O 指的是：调用某个 I/O 函数时，如果当前条件暂时不满足，函数不会让当前线程一直等待，而是立即返回，让程序自己决定下一步怎么办。

在网络编程里，最常见的场景是把 socket 设置成非阻塞：

- 没有连接可接收时，[[linux网络编程/函数笔记/Socket/accept|accept]] 不一直卡住。
- 没有数据可读时，[[linux网络编程/函数笔记/Socket/read|read]] / [[linux网络编程/函数笔记/Socket/recv|recv]] 不一直卡住。
- 当前不能继续写时，[[linux网络编程/函数笔记/Socket/send|send]] / `write` 不一直卡住。

## 和阻塞式 IO 的区别

| 对比项 | 阻塞式 IO | 非阻塞 IO |
|---|---|---|
| 条件不满足时 | 当前线程等待 | 立即返回 |
| 编程感受 | 顺序写法简单 | 需要自己处理“暂时做不了” |
| 并发处理 | 单线程容易卡住 | 更适合配合事件循环 |
| 常见搭配 | 阻塞式服务器、多进程、多线程 | [[linux网络编程/概念词条/IO多路复用|IO多路复用]]、[[linux网络编程/概念词条/Reactor反应堆模式|Reactor]]、边沿触发 |

对应的阻塞模型见 [[linux网络编程/概念词条/阻塞式IO|阻塞式IO]]。

## 为什么需要非阻塞 I/O

如果一个线程既要管理很多连接，又不想在某一个连接上被卡住，就不能让它在 `read/recv/accept` 上一直阻塞。

于是常见做法是：

```text
fd 设置为非阻塞
  ↓
IO 多路复用告诉我“这个 fd 现在可以读/写”
  ↓
我去读/写
  ↓
如果一次没处理完，就继续读/写
  ↓
直到返回 EAGAIN / EWOULDBLOCK
```

这就是为什么 [[linux网络编程/概念词条/epoll模型|epoll]] 尤其是 [[linux网络编程/概念词条/水平触发与边沿触发|边沿触发]] 常常要配合非阻塞 I/O。

## 如何设置非阻塞

最常见方法是用 [[linux系统编程/函数笔记/文件IO/fcntl|fcntl]]：

```c
int flags = fcntl(fd, F_GETFL, 0);
fcntl(fd, F_SETFL, flags | O_NONBLOCK);
```

这里：

- `F_GETFL`：先取出当前文件状态标志。
- `F_SETFL`：再把新标志写回去。
- `O_NONBLOCK`：表示把 fd 设为非阻塞。

相关概念：

- [[linux系统编程/概念词条/fcntl命令|fcntl命令]]
- [[linux系统编程/概念词条/open标志位|open标志位]]
- [[linux网络编程/概念词条/O_NONBLOCK|O_NONBLOCK]]

## 在网络编程中的表现

### accept

监听 socket 设为非阻塞后：

- 如果当前没有新连接，[[linux网络编程/函数笔记/Socket/accept|accept]] 会立刻返回 `-1`。
- 这时通常通过 [[linux网络编程/概念词条/EAGAIN与EWOULDBLOCK|EAGAIN / EWOULDBLOCK]] 判断“暂时没有连接”，而不是把它当成致命错误。

### read / recv

已连接 socket 设为非阻塞后：

- 如果接收缓冲区里暂时没有数据，[[linux网络编程/函数笔记/Socket/read|read]] / [[linux网络编程/函数笔记/Socket/recv|recv]] 会立即返回 `-1`。
- 并设置 `errno = EAGAIN` 或 `EWOULDBLOCK`。

### send / write

- 如果发送缓冲区当前不能继续写，也可能立即返回 `-1`，并设置 `errno` 为 `EAGAIN` 或 `EWOULDBLOCK`。

## 和 IO 多路复用的关系

单独把 fd 设成非阻塞，不代表程序自动高并发。

更常见的组合是：

- [[linux网络编程/函数笔记/IO多路复用/select|select]] / [[linux网络编程/函数笔记/IO多路复用/poll|poll]] / [[linux网络编程/函数笔记/IO多路复用/epoll_wait|epoll_wait]] 负责告诉你“哪个 fd 现在值得处理”。
- 非阻塞 I/O 负责保证你在真正处理这个 fd 时，不会因为一次没读完/没写完而把整个线程卡住。

所以可以这样记：

- IO 多路复用：告诉你“该处理谁”。
- 非阻塞 I/O：保证你“处理谁时不会被卡住”。

## 和边沿触发的关系

[[linux网络编程/概念词条/水平触发与边沿触发|边沿触发 ET]] 下，内核通常只在“状态变化”时通知一次。

因此常见写法是：

```text
fd 变成可读
  ↓
循环 read / recv
  ↓
直到返回 EAGAIN
```

如果 fd 还是阻塞模式，就可能：

- 某次读到一半卡住，拖住整个事件循环。
- 或者逻辑上不敢一直读，导致遗漏后续数据。

所以课程里常说：ET 模式通常要配合非阻塞 I/O。

## 易错点

- 非阻塞不等于“不会返回错误”。它只是把“等一等”变成“立刻返回，让你自己处理”。
- 返回 `-1` 不一定是严重错误，先看 `errno` 是否为 [[linux网络编程/概念词条/EAGAIN与EWOULDBLOCK|EAGAIN / EWOULDBLOCK]]。
- 设置非阻塞时不要直接 `F_SETFL, O_NONBLOCK` 覆盖旧标志，通常要先 `F_GETFL` 再按位或。
- 非阻塞 I/O 不是只属于 epoll，`accept/read/recv/send/connect` 等都可能涉及。

## 相关函数

- [[linux系统编程/函数笔记/文件IO/fcntl|fcntl]]
- [[linux网络编程/函数笔记/Socket/accept|accept]]
- [[linux网络编程/函数笔记/Socket/read|read]]
- [[linux网络编程/函数笔记/Socket/recv|recv]]
- [[linux网络编程/函数笔记/Socket/send|send]]
- [[linux网络编程/函数笔记/IO多路复用/epoll_wait|epoll_wait]]

## 相关概念

- [[linux网络编程/概念词条/阻塞式IO|阻塞式IO]]
- [[linux网络编程/概念词条/O_NONBLOCK|O_NONBLOCK]]
- [[linux网络编程/概念词条/EAGAIN与EWOULDBLOCK|EAGAIN与EWOULDBLOCK]]
- [[linux网络编程/概念词条/IO多路复用|IO多路复用]]
- [[linux网络编程/概念词条/水平触发与边沿触发|水平触发与边沿触发]]
- [[linux网络编程/概念词条/Reactor反应堆模式|Reactor（反应堆模式）]]

## 相关课时

- [[linux网络编程/课时笔记/05 IO多路复用/04 非阻塞I O|04 非阻塞I/O]]
- [[linux网络编程/课时笔记/05 IO多路复用/03 epoll|03 epoll]]

## 相关模块

- [[linux网络编程/05 IO多路复用|05 IO多路复用]]
