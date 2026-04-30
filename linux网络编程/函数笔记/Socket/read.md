---
title: read
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/Socket
---
# read

> [!info] 功能
> 从文件描述符读取数据。在网络编程中，socket 也是文件描述符，所以 `read` 可以直接用于 TCP 已连接 socket，效果类似基础用法下的 [[linux网络编程/函数笔记/Socket/recv|recv]]。

## 函数原型

- `ssize_t read(int fd, void *buf, size_t count);`

## 依赖头文件

- `#include <unistd.h>`

## 输入参数

- `fd`：要读取的文件描述符。在 TCP 网络编程中，服务器端通常是 [[linux网络编程/函数笔记/Socket/accept|accept]] 返回的 `connfd` / `cfd`，客户端通常是 [[linux网络编程/函数笔记/Socket/connect|connect]] 成功后的 socket fd。
- `buf`：用户提供的接收缓冲区首地址。`read` 会把从 socket 接收缓冲区读到的数据写入这里，因此这块内存必须可写，并且生命周期要覆盖本次调用。
- `count`：最多读取的字节数，类型是 [[linux网络编程/概念词条/size_t|size_t]]。它应该小于或等于 `buf` 实际可写空间，不能因为想多读而超过数组大小。

## 输出参数

- `buf`：成功读取时，保存实际读到的字节数据。注意 `read` 不会自动在末尾补 `\0`，如果要按 C 字符串处理，需要自己预留空间并补结尾。

## 返回值

- 成功返回实际读取的字节数，类型是 [[linux网络编程/概念词条/ssize_t|ssize_t]]。
- 返回 `0`：在 TCP socket 中，通常表示对端已经正常关闭写方向，也就是收到了对端 FIN；这和 [[linux网络编程/概念词条/TCP四次挥手|TCP四次挥手]]、[[linux网络编程/概念词条/TCP半关闭|TCP半关闭]] 有关。
- 返回 `-1`：读取失败，并设置 `errno`。常见原因包括信号中断、非阻塞模式下暂时无数据、fd 无效等。
-  为什么不可能“读 0 字节”？
	### TCP 是流协议
	    如果：
	        连接正常
	        没有数据
	        
	    那么：
	✅ 在 **阻塞模式**：
	    会 **阻塞等待数据**
	✅ 在 **非阻塞模式**：
	    返回 `-1`
	永远不会返回0

## 网络编程重点

- 对 TCP socket 来说，`read` 读的是内核 socket 接收缓冲区中的字节流。
- TCP 是 [[linux网络编程/概念词条/TCP|字节流协议]]，一次 `read` 不保证对应对端一次 `write/send`。
- 阻塞 socket 上，如果接收缓冲区没有数据，`read` 默认会阻塞等待。
- 如果对端调用 `close` 或 [[linux网络编程/函数笔记/Socket/shutdown|shutdown]] 关闭写方向，本端读完剩余数据后，`read` 通常返回 `0`。
- `read` 没有 `flags` 参数，如果需要 `MSG_PEEK`、`MSG_DONTWAIT` 等 socket 专用标志，应使用 [[linux网络编程/函数笔记/Socket/recv|recv]]。

## 常见用法

```c
char buf[1024];
ssize_t n = read(connfd, buf, sizeof(buf));
if (n > 0) {
    /* 处理 buf[0..n-1] */
} else if (n == 0) {
    /* 对端正常关闭写方向 */
} else {
    /* 出错处理 */
}
```

## 和 recv 的区别

| 对比项 | read | recv |
|---|---|---|
| 适用对象 | 任意文件描述符，包括 socket、管道、文件等 | socket 专用 |
| 函数参数 | `fd, buf, count` | `sockfd, buf, len, flags` |
| 是否支持 flags | 不支持 | 支持，例如 `MSG_PEEK`、`MSG_DONTWAIT` |
| 基础 TCP 读取 | 可以用 | 可以用 |

## 易错点

- 不能把返回值忽略掉；返回多少字节，就只能处理多少字节。
- 返回 `0` 不是“读到了空字符串”，而是 TCP 对端正常关闭写方向的重要信号。
- `buf` 不会自动变成字符串，打印前要考虑补 `\0`。
- TCP 不保留消息边界，应用层协议要自己处理粘包、半包。

## 相关概念

- [[linux网络编程/概念词条/TCP|TCP]]
- [[linux网络编程/概念词条/阻塞式IO|阻塞式IO]]
- [[linux网络编程/概念词条/已连接套接字|已连接套接字]]
- [[linux网络编程/概念词条/TCP四次挥手|TCP四次挥手]]
- [[linux网络编程/概念词条/TCP半关闭|TCP半关闭]]
- [[linux网络编程/概念词条/size_t|size_t]]
- [[linux网络编程/概念词条/ssize_t|ssize_t]]

## 相关函数

- [[linux网络编程/函数笔记/Socket/recv|recv]]
- [[linux网络编程/函数笔记/Socket/send|send]]
- [[linux网络编程/函数笔记/Socket/shutdown|shutdown]]
- [[linux网络编程/函数笔记/Socket/accept|accept]]
- [[linux网络编程/函数笔记/Socket/connect|connect]]

## 相关课时

- [[linux网络编程/课时笔记/03 TCP通信与通信案例/01 TCP通信基础案例|01 TCP通信基础案例]]
- [[linux网络编程/课时笔记/03 TCP通信与通信案例/02 客户端与服务器通信流程|02 客户端与服务器通信流程]]

## 相关模块

- [[linux网络编程/03 TCP通信与通信案例|03 TCP通信与通信案例]]
