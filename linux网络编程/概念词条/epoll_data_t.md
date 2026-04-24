---
title: epoll_data_t
tags:
  - linux
  - 网络编程
  - 概念词条
  - 网络编程/IO多路复用
---
# epoll_data_t

## 它是什么

[[linux网络编程/概念词条/epoll_data_t|epoll_data_t]] 是 `struct epoll_event` 里的用户数据类型，用来让应用程序在事件就绪返回时找回自己关心的信息。

最常见的用途是保存文件描述符：

```c
ev.data.fd = cfd;
```

这样 [[linux网络编程/函数笔记/IO多路复用/epoll_wait|epoll_wait]] 返回后，就可以通过 `events[i].data.fd` 知道哪个 fd 就绪。

## 依赖头文件

```c
#include <sys/epoll.h>
```

## 常见定义

在 Linux 中它通常是一个联合体：

```c
typedef union epoll_data {
    void     *ptr;
    int       fd;
    uint32_t  u32;
    uint64_t  u64;
} epoll_data_t;
```

不同系统或头文件展示可能略有差异，但核心点是：它可以保存几种不同形式的用户数据。

## 为什么是 union

`union` 表示多个成员共用同一块内存。

所以 [[linux网络编程/概念词条/epoll_data_t|epoll_data_t]] 一次通常只应该使用一个成员：

- `fd`：保存文件描述符，课程和入门代码最常用。
- `ptr`：保存指针，工程中常用来指向连接对象或业务上下文。
- `u32`：保存 32 位无符号整数。
- `u64`：保存 64 位无符号整数。

不要同时依赖 `fd` 和 `ptr` 的值，因为写入一个成员可能覆盖另一个成员的解释。

## 在 epoll_event 中的位置

```c
struct epoll_event {
    uint32_t events;
    epoll_data_t data;
};
```

- `events`：关注或返回的事件类型。
- `data`：用户自定义数据，类型就是 [[linux网络编程/概念词条/epoll_data_t|epoll_data_t]]。

## 常见用法：保存 fd

注册监听 fd：

```c
struct epoll_event ev;
ev.events = EPOLLIN;
ev.data.fd = lfd;

epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);
```

等待返回后取出 fd：

```c
struct epoll_event events[1024];
int nready = epoll_wait(epfd, events, 1024, -1);

for (int i = 0; i < nready; ++i) {
    int fd = events[i].data.fd;
}
```

## 常见用法：保存指针

复杂服务器中可能把连接信息封装成结构体：

```c
struct Conn {
    int fd;
    char buf[4096];
};

struct Conn *conn = malloc(sizeof(struct Conn));
conn->fd = cfd;

ev.events = EPOLLIN;
ev.data.ptr = conn;
epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
```

返回时再取回：

```c
struct Conn *conn = events[i].data.ptr;
```

课程入门阶段通常先掌握 `data.fd`，理解 `data.ptr` 是后续工程化写法即可。

## 易错点

- `epoll_data_t` 不是事件类型，事件类型在 `events` 字段里，例如 `EPOLLIN`、`EPOLLOUT`。
- `epoll_data_t` 是用户数据，内核只是原样保存并在事件就绪时带回来。
- 如果注册时忘记设置 `ev.data.fd` 或 `ev.data.ptr`，返回时就无法方便定位对应连接。
- 因为它是 `union`，一次通常只使用一个成员，不要同时写 `fd` 又指望 `ptr` 仍然有效。

## 相关函数

- [[linux网络编程/函数笔记/IO多路复用/epoll_ctl|epoll_ctl]]
- [[linux网络编程/函数笔记/IO多路复用/epoll_wait|epoll_wait]]

## 相关概念

- [[linux网络编程/概念词条/epoll_event|epoll_event]]
- [[linux网络编程/概念词条/epoll模型|epoll模型]]
- [[linux网络编程/概念词条/事件就绪|事件就绪]]

## 相关课时

- [[linux网络编程/课时笔记/05 IO多路复用/03 epoll|03 epoll]]

## 相关模块

- [[linux网络编程/05 IO多路复用|05 IO多路复用]]
