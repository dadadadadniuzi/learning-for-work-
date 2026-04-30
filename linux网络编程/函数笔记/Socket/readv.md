---
title: readv
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/Socket
---
# readv

> [!info] 功能
> 从一个文件描述符中分散读取数据到多个缓冲区。网络编程里常用于一次把数据读进多段内存。

## 函数原型

- `ssize_t readv(int fd, const struct iovec *iov, int iovcnt);`

## 依赖头文件

- `#include <sys/uio.h>`
- `#include <unistd.h>`

## 输入参数

- `fd`：要读取的文件描述符。在网络编程中通常是已连接 socket，也可以是普通文件、管道等。
- `iov`：指向一组 [[linux网络编程/概念词条/iovec|iovec]] 结构体的首地址。每个 `iovec` 描述一段可写缓冲区，`readv` 会按顺序把读到的数据依次写入这些缓冲区。
- `iovcnt`：`iov` 数组中的元素个数。它表示这次读取一共要分散写入多少段缓冲区。

## 输出参数

- `iov` 指向的各段缓冲区：成功时保存读取到的数据。

## 返回值

- 成功返回实际读取的总字节数，类型是 [[linux网络编程/概念词条/ssize_t|ssize_t]]。
- 返回 `0`：对 socket 来说，通常表示对端已经正常关闭写方向。
- 返回 `-1`：读取失败，并设置 `errno`。

## 怎么理解

普通 `read` 只能把数据读进一块连续缓冲区。

`readv` 则可以一次读进多块缓冲区，例如：

- 前 4 个字节放到头部缓冲区
- 后面的数据放到正文缓冲区

这样就不用先读到一块大缓冲区里，再手动拆分。

## 常见用法

```c
#include <sys/uio.h>
#include <unistd.h>

char head[4];
char body[128];

struct iovec iv[2];
iv[0].iov_base = head;
iv[0].iov_len = sizeof(head);
iv[1].iov_base = body;
iv[1].iov_len = sizeof(body);

ssize_t n = readv(fd, iv, 2);
```

## 典型场景

- 协议头和消息体分开接收
- 多段缓冲区协同读入
- 减少“先读后拆”的额外拷贝

## 易错点

- `iov` 中每一段缓冲区都必须可写。
- 返回值是“总共读了多少字节”，不是某一段单独的长度。
- 如果读到的数据不足填满全部 `iovec`，后面的缓冲区可能只填一部分，甚至完全没写到。
- 它和普通 `read` 一样，也可能遇到短读。

## 和相关函数的区别

- `read`：读到一段连续缓冲区。
- `readv`：读到多段缓冲区。
- `writev`：把多段缓冲区一次性写出去。

## 相关概念

- [[linux网络编程/概念词条/iovec|iovec]]
- [[linux网络编程/概念词条/size_t|size_t]]
- [[linux网络编程/概念词条/ssize_t|ssize_t]]
- [[linux网络编程/概念词条/TCP|TCP]]

## 相关函数

- [[linux网络编程/函数笔记/Socket/read|read]]
- [[linux网络编程/函数笔记/Socket/writev|writev]]
- [[linux网络编程/函数笔记/Socket/recv|recv]]

## 相关模块

- [[linux网络编程/03 TCP通信与通信案例|03 TCP通信与通信案例]]
- [[linux网络编程/04 高并发服务器|04 高并发服务器]]
