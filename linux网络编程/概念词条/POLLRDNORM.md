---
title: POLLRDNORM
tags:
  - linux
  - 网络编程
  - 概念词条
  - 网络编程/IO多路复用
---
# POLLRDNORM

## 它是什么

[[linux网络编程/概念词条/POLLRDNORM|POLLRDNORM]] 是 [[linux网络编程/函数笔记/IO多路复用/poll|poll]] 使用的事件宏，含义是“普通数据可读”。

它通常写在 [[linux网络编程/概念词条/pollfd|struct pollfd]] 的 `events` 字段中，表示应用程序希望监听普通数据是否可读；也可能出现在 `revents` 字段中，表示内核告诉应用程序普通数据已经可读。

## 依赖头文件

```c
#include <poll.h>
```

## 和 POLLIN 的关系

在 Linux 网络编程学习中，可以先这样记：

- `POLLIN`：泛指可读事件。
- [[linux网络编程/概念词条/POLLRDNORM|POLLRDNORM]]：强调“普通数据可读”。

对普通 TCP socket 来说，课程里遇到 [[linux网络编程/概念词条/POLLRDNORM|POLLRDNORM]] 时，通常可以把它理解成和 `POLLIN` 很接近的可读事件。区别在于命名更细：`RDNORM` 表示 read normal data，即读取普通数据。

## 常见用法

监听普通数据可读：

```c
struct pollfd pfd;
pfd.fd = cfd;
pfd.events = POLLRDNORM;
pfd.revents = 0;
```

判断普通数据是否可读：

```c
if (pfd.revents & POLLRDNORM) {
    char buf[1024];
    int n = read(pfd.fd, buf, sizeof(buf));
}
```

也可以和其他事件组合：

```c
pfd.events = POLLRDNORM | POLLOUT;
```

## 在服务器中的含义

- 如果通信 socket 出现 [[linux网络编程/概念词条/POLLRDNORM|POLLRDNORM]]，通常表示可以调用 [[linux网络编程/函数笔记/Socket/read|read]] 或 [[linux网络编程/函数笔记/Socket/recv|recv]] 读取普通数据。
- 如果读取返回 `0`，表示对端关闭连接。
- 如果读取返回大于 `0`，表示读到了数据。
- 如果读取返回 `-1`，需要结合 `errno` 判断错误原因。

## 易错点

- 判断事件时要用按位与：`revents & POLLRDNORM`，不要写成 `revents == POLLRDNORM`。
- [[linux网络编程/概念词条/POLLRDNORM|POLLRDNORM]] 不是函数，也不是结构体字段，它是事件标志宏。
- 对初学阶段来说，看到 [[linux网络编程/概念词条/POLLRDNORM|POLLRDNORM]] 可以先归入“可读事件”这类，再进一步记住它强调普通数据。

## 相关函数

- [[linux网络编程/函数笔记/IO多路复用/poll|poll]]
- [[linux网络编程/函数笔记/Socket/read|read]]
- [[linux网络编程/函数笔记/Socket/recv|recv]]

## 相关概念

- [[linux网络编程/概念词条/poll事件宏|poll事件宏]]
- [[linux网络编程/概念词条/pollfd|struct pollfd]]
- [[linux网络编程/概念词条/poll模型|poll模型]]
- [[linux网络编程/概念词条/事件就绪|事件就绪]]

## 相关课时

- [[linux网络编程/课时笔记/05 IO多路复用/02 poll|02 poll]]

## 相关模块

- [[linux网络编程/05 IO多路复用|05 IO多路复用]]
