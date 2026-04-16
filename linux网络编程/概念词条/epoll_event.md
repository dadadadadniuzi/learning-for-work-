---
title: epoll_event
tags:
  - linux
  - 网络编程
  - 概念词条
---
# epoll_event

## 它是什么

- `struct epoll_event` 是 epoll 用来描述关注事件和返回就绪事件的结构体。

## 常见原型

```c
struct epoll_event {
    uint32_t events;
    epoll_data_t data;
};
```

## 字段说明

- `events`：事件类型，例如 `EPOLLIN` 表示可读，`EPOLLOUT` 表示可写，`EPOLLET` 表示边沿触发。
- `data`：用户数据，常用 `data.fd` 保存对应的文件描述符。

## 怎么理解

- 注册事件时，把 fd 和关心的事件打包成 `epoll_event` 交给 `epoll_ctl`。
- 等待事件时，`epoll_wait` 把已经就绪的事件写入 `epoll_event` 数组。

## 易错点

- `data.fd` 常用于保存 fd，但 `data` 也可以保存指针等其他用户数据。
- `events` 既可以描述关注事件，也可以在返回数组里描述实际就绪事件。

## 常见出现位置

- [[linux网络编程/函数笔记/IO多路复用/epoll_ctl|epoll_ctl]]
- [[linux网络编程/函数笔记/IO多路复用/epoll_wait|epoll_wait]]
- [[linux网络编程/课时笔记/05 IO多路复用/03 epoll|03 epoll]]
