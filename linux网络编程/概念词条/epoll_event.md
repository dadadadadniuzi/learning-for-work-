---
title: epoll_event
tags:
  - linux
  - 网络编程
  - 概念词条
  - 网络编程/IO多路复用
---
# epoll_event

## 它是什么

`struct epoll_event` 是 [[linux网络编程/概念词条/epoll模型|epoll模型]] 用来描述“关注什么事件”和“返回什么就绪事件”的结构体。

它会出现在两个关键位置：

- 调用 [[linux网络编程/函数笔记/IO多路复用/epoll_ctl|epoll_ctl]] 时，把要关注的 fd 和事件打包注册到 epoll 实例中。
- 调用 [[linux网络编程/函数笔记/IO多路复用/epoll_wait|epoll_wait]] 时，内核把已经就绪的事件写入 `struct epoll_event` 数组。

## 依赖头文件

```c
#include <sys/epoll.h>
```

## 常见原型

```c
struct epoll_event {
    uint32_t events;
    epoll_data_t data;
};
```

## 字段说明

- `events`：事件类型。注册时表示希望关注什么事件，返回时表示实际发生了什么事件。
- `data`：用户数据，类型是 [[linux网络编程/概念词条/epoll_data_t|epoll_data_t]]。最常用的是 `data.fd`，用来保存对应的文件描述符。

## events 常见取值

- `EPOLLIN`：可读。监听 socket 上通常表示有新连接可 [[linux网络编程/函数笔记/Socket/accept|accept]]，通信 socket 上通常表示可 [[linux网络编程/函数笔记/Socket/read|read]] 或 [[linux网络编程/函数笔记/Socket/recv|recv]]。
- `EPOLLOUT`：可写。
- `EPOLLERR`：错误。
- `EPOLLHUP`：挂起。
- `EPOLLET`：边沿触发，和 [[linux网络编程/概念词条/水平触发与边沿触发|水平触发与边沿触发]] 有关。

## data 常见用法

最常见写法是保存 fd：

```c
struct epoll_event ev;
ev.events = EPOLLIN;
ev.data.fd = lfd;

epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);
```

之后 [[linux网络编程/函数笔记/IO多路复用/epoll_wait|epoll_wait]] 返回时，可以通过 `events[i].data.fd` 判断哪个 fd 就绪：

```c
int fd = events[i].data.fd;
```

更复杂的服务器也可能使用 `data.ptr` 保存业务结构体指针，例如连接对象、客户端上下文等。

## epoll_ctl 和 epoll_wait 中的区别

| 场景             | struct epoll_event 的作用              |     |
| -------------- | ----------------------------------- | --- |
| [[epoll_ctl]]  | 输入参数，告诉内核要监听哪个 fd、关心哪些事件、返回时带什么用户数据 |     |
| [[epoll_wait]] | 输出数组，内核把已经就绪的事件写到数组中                |     |

## 易错点

- 注册时如果没有给 `data.fd` 赋值，[[linux网络编程/函数笔记/IO多路复用/epoll_wait|epoll_wait]] 返回后就很难判断哪个 fd 就绪。
- `events` 既用于注册关注事件，也用于返回就绪事件，读代码时要看它处在 [[linux网络编程/函数笔记/IO多路复用/epoll_ctl|epoll_ctl]] 之前还是 [[linux网络编程/函数笔记/IO多路复用/epoll_wait|epoll_wait]] 之后。
- [[linux网络编程/概念词条/epoll_data_t|epoll_data_t]] 是联合体，一次通常只使用其中一个成员，例如 `fd` 或 `ptr`，不要指望同时保存多个成员。

## 相关函数

- [[linux网络编程/函数笔记/IO多路复用/epoll_ctl|epoll_ctl]]
- [[linux网络编程/函数笔记/IO多路复用/epoll_wait|epoll_wait]]
- [[linux网络编程/函数笔记/IO多路复用/epoll_create|epoll_create]]

## 相关概念

- [[linux网络编程/概念词条/epoll_data_t|epoll_data_t]]
- [[linux网络编程/概念词条/epoll模型|epoll模型]]
- [[linux网络编程/概念词条/事件就绪|事件就绪]]
- [[linux网络编程/概念词条/水平触发与边沿触发|水平触发与边沿触发]]

## 相关课时

- [[linux网络编程/课时笔记/05 IO多路复用/03 epoll|03 epoll]]

## 相关模块

- [[linux网络编程/05 IO多路复用|05 IO多路复用]]
