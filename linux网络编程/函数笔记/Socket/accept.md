---
title: accept
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/Socket
---
# accept

> [!info] 功能
> 阻塞地从监听队列中取出一个客户端连接，**返回用于通信的新 socket**。

## 函数原型

- `int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);`

## 依赖头文件

- `#include <sys/types.h>`
- `#include <sys/socket.h>`

## 输入参数

- `sockfd`：监听套接字文件描述符，通常是 `socket -> bind -> listen` 后的 fd。
- `addr`：用于**接收客户端地址信息**的缓冲区地址。IPv4 中通常准备 [[linux网络编程/概念词条/sockaddr_in|sockaddr_in]]，再转换成 `struct sockaddr *`。如果不关心客户端地址，可以传 `NULL`。
- `addrlen`：传入传出参数。调用前要填入 `addr` 指向的地址结构大小；返回后，内核会把它改成实际写入的客户端地址长度。如果 `addr` 为 `NULL`，通常这里也传 `NULL`。

## 输出参数

- `addr`：成功时保存客户端地址结构。
- `addrlen`：成功时保存实际客户端地址长度。

## 返回值

- 成功返回新的已连接 socket 文件描述符。
- 失败返回 `-1`，并设置 `errno`。

## 知识点补充

- `accept` 默认会阻塞等待客户端连接。
- **返回的新 fd 是**[[linux网络编程/概念词条/已连接套接字|已连接套接字]]，用于和该客户端通信。
- **原来的 `sockfd` 仍然是**[[linux网络编程/概念词条/监听套接字|监听套接字]]，可以继续等待其他客户端。

## 常见用法

```c
struct sockaddr_in cliaddr;
socklen_t len = sizeof(cliaddr);
int cfd = accept(lfd, (struct sockaddr *)&cliaddr, &len);
```

## 易错点

- 第三个参数要传 `&len`，不是 `len`。
- `accept` 返回的 `cfd` 才用于读写客户端数据。
- 每次调用前最好重新设置 `len = sizeof(cliaddr)`。

## 相关概念

- [[linux网络编程/概念词条/监听套接字|监听套接字]]
- [[linux网络编程/概念词条/已连接套接字|已连接套接字]]
- [[linux网络编程/概念词条/sockaddr|sockaddr]]
- [[linux网络编程/概念词条/socklen_t|socklen_t]]
- [[linux网络编程/概念词条/Unix Domain Socket|Unix Domain Socket]]

## 相关课时

- [[linux网络编程/课时笔记/02 Socket编程基础/01 套接字与socket模型|01 套接字与socket模型]]
- [[linux网络编程/课时笔记/02 Socket编程基础/05 socket-bind-listen-accept-connect|05 socket-bind-listen-accept-connect]]
- [[linux网络编程/课时笔记/06 本地套接字与通信总结/01 Unix Domain Socket基础|01 Unix Domain Socket基础]]

## 相关模块

- [[linux网络编程/02 Socket编程基础|02 Socket编程基础]]
- [[linux网络编程/06 本地套接字与通信总结|06 本地套接字与通信总结]]
