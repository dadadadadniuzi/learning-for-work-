---
title: recvfrom
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/Socket
---
# recvfrom

> [!info] 功能
> 从 socket 接收数据，并可同时获取发送方地址。UDP 编程中最常用。

## 函数原型

- `ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);`

## 依赖头文件

- `#include <sys/types.h>`
- `#include <sys/socket.h>`

## 输入参数

- `sockfd`：socket 文件描述符。UDP 服务器通常先 [[linux网络编程/函数笔记/Socket/bind|bind]] 到固定端口，再调用 [[linux网络编程/函数笔记/Socket/recvfrom|recvfrom]]。
- `buf`：接收缓冲区首地址，用来保存收到的数据。
- `len`：接收缓冲区大小，类型是 [[linux网络编程/概念词条/size_t|size_t]]。最多接收这么多字节。
- `flags`：接收标志。基础 UDP 通信中通常传 `0`。
- `src_addr`：保存发送方地址的缓冲区指针。如果不关心来源地址，可以传 `NULL`。
- `addrlen`：输入时表示 `src_addr` 指向的地址缓冲区大小；输出时被内核改写为实际地址长度。类型是 [[linux网络编程/概念词条/socklen_t|socklen_t]] 指针。如果 `src_addr` 传 `NULL`，这里也通常传 `NULL`。

## 输出参数

- `buf`：保存接收到的数据。
- `src_addr`：保存发送方地址，UDP 服务器通常用它知道客户端是谁。
- `addrlen`：保存实际写入的地址长度。

## 返回值

- 成功返回实际接收的字节数。
- 失败返回 `-1`，并设置 `errno`。

## UDP 常见用法

```c
char buf[1024];
struct sockaddr_in cliaddr;
socklen_t len = sizeof(cliaddr);

ssize_t n = recvfrom(fd, buf, sizeof(buf), 0,
                     (struct sockaddr *)&cliaddr, &len);
```

服务器回复客户端时，常把 `cliaddr` 交给 [[linux网络编程/函数笔记/Socket/sendto|sendto]]：

```c
sendto(fd, buf, n, 0, (struct sockaddr *)&cliaddr, len);
```

## 在 UDP 通信中的作用

UDP 没有 [[linux网络编程/函数笔记/Socket/accept|accept]] 返回的已连接 socket。服务器通常通过同一个 UDP socket 接收多个客户端的数据。

因此 [[linux网络编程/函数笔记/Socket/recvfrom|recvfrom]] 的来源地址输出参数很关键：它告诉服务器“这份数据报是谁发来的”。

## 和 recv 的区别

- [[linux网络编程/函数笔记/Socket/recv|recv]]：常用于已连接 socket，不返回发送方地址。
- [[linux网络编程/函数笔记/Socket/recvfrom|recvfrom]]：可以返回发送方地址，UDP 中更常见。

## 易错点

- 调用前要初始化 `socklen_t len = sizeof(cliaddr);`。
- `addrlen` 要传地址，即 `&len`，不是 `len`。
- UDP 有数据报边界。如果缓冲区太小，超出部分可能被截断，后续不能像 TCP 那样继续读同一个数据报剩余部分。
- `recvfrom` 返回的数据不一定以 `\0` 结尾，如果要当字符串打印，需要自己补 `\0`。

## 相关函数

- [[linux网络编程/函数笔记/Socket/sendto|sendto]]
- [[linux网络编程/函数笔记/Socket/socket|socket]]
- [[linux网络编程/函数笔记/Socket/bind|bind]]
- [[linux网络编程/函数笔记/Socket/recv|recv]]

## 相关概念

- [[linux网络编程/概念词条/UDP|UDP]]
- [[linux网络编程/概念词条/UDP通信流程|UDP通信流程]]
- [[linux网络编程/概念词条/UDP数据报格式|UDP数据报格式]]
- [[linux网络编程/概念词条/sockaddr_in|sockaddr_in]]
- [[linux网络编程/概念词条/socklen_t|socklen_t]]

## 相关课时

- [[linux网络编程/课时笔记/03 TCP通信与通信案例/03 UDP通信案例|03 UDP通信案例]]

## 相关模块

- [[linux网络编程/03 TCP通信与通信案例|03 TCP通信与通信案例]]
