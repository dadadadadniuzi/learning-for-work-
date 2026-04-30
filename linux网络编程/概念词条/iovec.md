---
title: iovec
tags:
  - linux
  - 网络编程
  - 概念词条
---
# iovec

## 是什么

`iovec` 是 Linux 中“分散读写”常用的结构体，用来描述“一段内存区域”的起始地址和长度。

它最常见的用途是配合 `readv`、`writev` 这样的函数，一次处理多段缓冲区。

## 常见定义

```c
struct iovec {
    void  *iov_base;
    size_t iov_len;
};
```

## 依赖头文件

- `#include <sys/uio.h>`

## 字段解释

- `iov_base`
  这一段内存的起始地址。
  如果是发送数据，它指向要发送的数据起点。
  如果是接收数据，它指向要写入的数据缓冲区起点。

- `iov_len`
  这一段内存的长度，单位是字节。

## 怎么理解

可以把 `iovec` 理解成“一个缓冲区说明书”。

如果程序要发送两段数据：

- 第一段是 HTTP 响应头
- 第二段是文件内容

那么就可以准备两个 `iovec`：

```c
struct iovec iv[2];

iv[0].iov_base = write_buf;
iv[0].iov_len = write_len;

iv[1].iov_base = file_addr;
iv[1].iov_len = file_len;
```

然后交给 `writev()` 一次性发送。

## 在网络编程里为什么常见

`iovec` 常出现在高性能网络程序中，因为它适合“多段数据一起发”：

- 响应头一段
- 消息体一段
- 文件内容一段

这样就不一定要先把它们手动拷贝到一个大缓冲区里，再调用一次 `write`。

## 和 `writev` 的关系

- `write`：一次发送一段连续内存
- [[linux网络编程/函数笔记/Socket/writev|writev]]：一次发送多段内存
- [[linux网络编程/函数笔记/Socket/readv|readv]]：一次读取到多段内存
- `iovec`：就是给 `readv` / `writev` 描述每一段内存用的结构体

## TinyWebServer 里的典型场景

在 TinyWebServer 这类项目中，经常会看到：

- `iovec[0]` 存响应头
- `iovec[1]` 存 `mmap` 后的文件内容

然后通过 `writev` 一次把“响应头 + 文件内容”发给浏览器。

这比“先拼接再发送”更高效。

## 易错点

- `iov_len` 的单位是字节，不是元素个数。
- `iov_base` 指向的内存在发送期间必须保持有效。
- 如果底层是非阻塞 socket，`writev` 也可能只发送一部分，需要继续处理剩余数据。
- `iovec` 只是“描述缓冲区”，它自己不负责分配内存。

## 相关笔记

- [[linux网络编程/概念词条/size_t|size_t]]
- [[linux网络编程/概念词条/ssize_t|ssize_t]]
- [[linux网络编程/概念词条/非阻塞I O|非阻塞I/O]]
- [[linux网络编程/概念词条/TCP通信流程|TCP通信流程]]
- [[linux网络编程/函数笔记/Socket/readv|readv]]
- [[linux网络编程/函数笔记/Socket/writev|writev]]
