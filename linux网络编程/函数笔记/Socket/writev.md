---
title: writev
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/Socket
---
# writev

> [!info] 功能
> 把多段缓冲区的数据一次性写到同一个文件描述符。网络编程里常用于一次发送“响应头 + 响应体”。

## 函数原型

- `ssize_t writev(int fd, const struct iovec *iov, int iovcnt);`

## 依赖头文件

- `#include <sys/uio.h>`
- `#include <unistd.h>`

## 输入参数

- `fd`：要写入的文件描述符。在网络编程中通常是已连接 socket，也可以是文件、管道等。
- `iov`：指向一组 [[linux网络编程/概念词条/iovec|iovec]] 结构体的首地址。每个 `iovec` 描述一段待发送内存。
- `iovcnt`：`iov` 数组中的元素个数，表示这次一共要发送多少段缓冲区。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回实际写出的总字节数，类型是 [[linux网络编程/概念词条/ssize_t|ssize_t]]。
- 返回值可能小于所有 `iov_len` 之和，这叫短写。
- 失败返回 `-1`，并设置 `errno`。

## 怎么理解

普通 `write` 一次只能发送一段连续内存。

`writev` 可以一次发送多段，例如：

- 第一段是 HTTP 响应头
- 第二段是文件内容

这样程序就不需要先把两段内容手动拼成一个大缓冲区，再调用一次 `write`。

## 常见用法

```c
#include <sys/uio.h>
#include <unistd.h>

struct iovec iv[2];
iv[0].iov_base = write_buf;
iv[0].iov_len = write_len;
iv[1].iov_base = file_addr;
iv[1].iov_len = file_len;

ssize_t n = writev(fd, iv, 2);
```

## 典型场景

- HTTP 响应头和文件内容一起发送
- 多段缓冲区零拷贝风格发送
- 高性能服务器减少一次内存拼接

## 网络编程重点

- 对 TCP 来说，`writev` 写出去的仍然是字节流，不保留消息边界。
- 非阻塞 socket 下，`writev` 也可能只发出一部分数据。
- 如果发生短写，程序要正确更新 `iovec` 剩余发送位置，而不是简单重发全部内容。

## TinyWebServer 里的典型场景

- `iovec[0]` 存响应头
- `iovec[1]` 存 `mmap` 后的静态文件内容
- 再通过 `writev` 一次发给浏览器

这就是常说的 `mmap + writev` 路径。

## 易错点

- `iov_base` 指向的内存必须在发送期间保持有效。
- 短写后不能直接当作“全部发送成功”。
- `iovcnt` 是段数，不是总字节数。
- `writev` 只负责发送，不会帮你自动拼接协议边界。

## 和相关函数的区别

- `write`：发送一段连续缓冲区。
- `writev`：一次发送多段缓冲区。
- `send`：socket 专用，支持 `flags`。

## 相关概念

- [[linux网络编程/概念词条/iovec|iovec]]
- [[linux网络编程/概念词条/size_t|size_t]]
- [[linux网络编程/概念词条/ssize_t|ssize_t]]
- [[linux网络编程/概念词条/非阻塞I O|非阻塞I/O]]
- [[linux网络编程/概念词条/TCP|TCP]]

## 相关函数

- [[linux网络编程/函数笔记/Socket/send|send]]
- [[linux网络编程/函数笔记/Socket/readv|readv]]
- [[linux网络编程/函数笔记/Socket/read|read]]

## 相关模块

- [[linux网络编程/03 TCP通信与通信案例|03 TCP通信与通信案例]]
- [[linux网络编程/04 高并发服务器|04 高并发服务器]]
