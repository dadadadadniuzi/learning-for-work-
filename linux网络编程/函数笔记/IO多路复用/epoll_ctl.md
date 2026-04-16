---
title: epoll_ctl
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/IO多路复用
---
# epoll_ctl

> [!info] 功能
> 控制 epoll 实例中的监听项，添加、修改或删除被关注的 fd。

## 函数原型

- `int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);`

## 依赖头文件

- `#include <sys/epoll.h>`

## 输入参数

- `epfd`：`epoll_create` 返回的 epoll 文件描述符。
- `op`：操作类型。`EPOLL_CTL_ADD` 添加 fd，`EPOLL_CTL_MOD` 修改 fd，`EPOLL_CTL_DEL` 删除 fd。
- `fd`：要添加、修改或删除的目标文件描述符，例如监听 socket 或已连接 socket。
- `event`：事件结构指针。添加或修改时用于描述关注事件和用户数据；删除时在很多 Linux 实现中可传 `NULL`。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回 `0`。
- 失败返回 `-1`。

## 知识点补充

- 常见关注读事件写法是 `ev.events = EPOLLIN; ev.data.fd = fd;`。
- 监听 fd 和通信 fd 都可以注册进同一个 epoll 实例。

## 常见用法

```c
struct epoll_event ev;
ev.events = EPOLLIN;
ev.data.fd = lfd;
epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);
```

## 易错点

- 添加 fd 后，如果不再关注，要记得删除或关闭 fd。
- `event.data.fd` 通常要保存目标 fd，否则 `epoll_wait` 返回后不好判断是谁就绪。

## 相关概念

- [[linux网络编程/概念词条/epoll模型|epoll模型]]
- [[linux网络编程/概念词条/epoll_event|epoll_event]]

## 相关课时

- [[linux网络编程/课时笔记/05 IO多路复用/03 epoll|03 epoll]]

## 相关模块

- [[linux网络编程/05 IO多路复用|05 IO多路复用]]
