---
title: poll事件宏
tags:
  - linux
  - 网络编程
  - 概念词条
  - 网络编程/IO多路复用
---
# poll事件宏

## 它是什么

poll 事件宏是写入 [[linux网络编程/概念词条/pollfd|struct pollfd]] 的 `events` 字段、或从 `revents` 字段读取的事件标志。

这些宏本质上是位标志，可以通过按位或组合，也可以通过按位与判断。

## 依赖头文件

```c
#include <poll.h>
```

## 常见宏

- `POLLIN`：可读。有数据可读，或监听 socket 上有新连接可接收。
- [[linux网络编程/概念词条/POLLRDNORM|POLLRDNORM]]：普通数据可读。Linux 网络编程里通常可以近似理解为和 `POLLIN` 表示的普通可读事件一致。
- `POLLOUT`：可写。写缓冲区有空间，可以发送数据。
- `POLLERR`：错误。fd 上出现错误条件。
- `POLLHUP`：挂起。对端关闭或连接挂起。
- `POLLNVAL`：无效 fd。通常说明传入了错误文件描述符。

## 常见用法

监听可读事件：

```c
fds[i].events = POLLIN;
```

监听普通数据可读事件：

```c
fds[i].events = POLLRDNORM;
```

同时监听可读和可写：

```c
fds[i].events = POLLIN | POLLOUT;
```

判断是否可读：

```c
if (fds[i].revents & POLLIN) {
    // read / recv / accept
}
```

判断普通数据是否可读：

```c
if (fds[i].revents & POLLRDNORM) {
    // read / recv
}
```

## events 和 revents 中的区别

- `events`：应用程序写入，表示想监听什么。
- `revents`：内核写入，表示实际发生了什么。

有些事件即使没有写进 `events`，也可能出现在 `revents` 中，比如 `POLLERR`、`POLLHUP`、`POLLNVAL` 这类异常/错误状态。

## 易错点

- 判断事件时不要写成 `fds[i].revents == POLLIN`，因为 `revents` 里可能同时有多个标志；更常见写法是 `fds[i].revents & POLLIN`。
- `POLLIN` 在监听 socket 和通信 socket 上含义不同：监听 socket 可读表示可 [[linux网络编程/函数笔记/Socket/accept|accept]]，通信 socket 可读表示可 [[linux网络编程/函数笔记/Socket/read|read]] 或 [[linux网络编程/函数笔记/Socket/recv|recv]]。
- [[linux网络编程/概念词条/POLLRDNORM|POLLRDNORM]] 强调“普通数据可读”。课程或手册中看到它时，不要误以为它是新的一类 socket；它仍然是 [[linux网络编程/函数笔记/IO多路复用/poll|poll]] 的可读事件标志。
- 发现 `POLLERR`、`POLLHUP`、`POLLNVAL` 时通常要做错误处理或关闭对应 fd。

## 相关函数

- [[linux网络编程/函数笔记/IO多路复用/poll|poll]]
- [[linux网络编程/函数笔记/Socket/accept|accept]]
- [[linux网络编程/函数笔记/Socket/read|read]]
- [[linux网络编程/函数笔记/Socket/recv|recv]]

## 相关概念

- [[linux网络编程/概念词条/pollfd|struct pollfd]]
- [[linux网络编程/概念词条/poll模型|poll模型]]
- [[linux网络编程/概念词条/事件就绪|事件就绪]]
- [[linux网络编程/概念词条/POLLRDNORM|POLLRDNORM]]

## 相关课时

- [[linux网络编程/课时笔记/05 IO多路复用/02 poll|02 poll]]
