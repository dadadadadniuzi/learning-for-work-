---
title: struct pollfd
tags:
  - linux
  - 网络编程
  - 概念词条
  - 网络编程/IO多路复用
---
# struct pollfd

## 它是什么

[[linux网络编程/概念词条/pollfd|struct pollfd]] 是 [[linux网络编程/函数笔记/IO多路复用/poll|poll]] 使用的监听项结构。

[[linux网络编程/概念词条/select模型|select模型]] 使用 [[linux网络编程/概念词条/fd_set|fd_set]] 位图描述一组 fd；[[linux网络编程/概念词条/poll模型|poll模型]] 则使用 `struct pollfd` 数组，每个数组元素描述一个 fd 以及它关心和实际发生的事件。

## 依赖头文件

```c
#include <poll.h>
```

## 结构原型

```c
struct pollfd {
    int   fd;
    short events;
    short revents;
};
```

## 字段说明

- `fd`：要监听的文件描述符。可以是监听 socket，也可以是已连接 socket。
- `events`：输入字段，应用程序设置，表示“我关心这个 fd 上发生什么事件”。
- `revents`：输出字段，内核设置，表示“这次 [[linux网络编程/函数笔记/IO多路复用/poll|poll]] 返回时，这个 fd 实际发生了什么事件”。

## events 和 revents 的区别

| 字段 | 谁写 | 什么时候看 | 含义 |
|---|---|---|---|
| `events` | 应用程序 | 调用 [[linux网络编程/函数笔记/IO多路复用/poll|poll]] 前设置 | 想监听什么事件 |
| `revents` | 内核 | [[linux网络编程/函数笔记/IO多路复用/poll|poll]] 返回后检查 | 实际发生了什么事件 |

这比 [[linux网络编程/概念词条/fd_set|fd_set]] 更直观：`events` 不会被内核覆盖，返回结果写到 `revents`。

## 常见事件宏

- [[linux网络编程/概念词条/poll事件宏|POLLIN]]：可读。监听 socket 上通常表示有新连接可 [[linux网络编程/函数笔记/Socket/accept|accept]]；通信 socket 上通常表示有数据可读或对端关闭。
- [[linux网络编程/概念词条/POLLRDNORM|POLLRDNORM]]：普通数据可读。课程里如果和 `POLLIN` 一起出现，可以先记成“普通读事件”的更细分写法。
- [[linux网络编程/概念词条/poll事件宏|POLLOUT]]：可写。通常表示写缓冲区有空间，可以发送数据。
- [[linux网络编程/概念词条/poll事件宏|POLLERR]]：错误事件。
- [[linux网络编程/概念词条/poll事件宏|POLLHUP]]：挂起，常和连接关闭有关。
- [[linux网络编程/概念词条/poll事件宏|POLLNVAL]]：无效 fd。

## 常见代码

```c
struct pollfd fds[1024];

fds[0].fd = lfd;
fds[0].events = POLLIN;
fds[0].revents = 0;

int nready = poll(fds, 1, -1);
if (nready > 0 && (fds[0].revents & POLLIN)) {
    int cfd = accept(lfd, NULL, NULL);
}
```

## 不使用某个元素时怎么办

如果数组中某个位置暂时不用，常把它的 `fd` 设置为 `-1`：

```c
fds[i].fd = -1;
```

[[linux网络编程/函数笔记/IO多路复用/poll|poll]] 会忽略 `fd < 0` 的元素，并把对应的 `revents` 返回为 0。

## 和 fd_set 的区别

| 对比项 | fd_set | struct pollfd |
|---|---|---|
| 所属模型 | select 模型 | poll 模型 |
| 数据结构 | 位图集合 | 结构体数组 |
| 是否需要最大 fd + 1 | 需要 | 不需要 |
| 返回结果 | 原集合被改写 | 写入 `revents` |
| 容量限制 | 受 FD_SETSIZE 影响 | 不受 `fd_set` 位图大小限制，但数组仍由应用程序分配 |

相关跳转：

- [[linux网络编程/概念词条/fd_set|fd_set]]
- [[linux网络编程/概念词条/pollfd|struct pollfd]]
- [[linux网络编程/概念词条/select模型|select模型]]
- [[linux网络编程/概念词条/poll模型|poll模型]]
- [[linux网络编程/概念词条/FD_SETSIZE|FD_SETSIZE]]

## 易错点

- `events` 是调用前设置的，`revents` 是调用后检查的，不要反过来。
- 判断事件时通常要用位与：`fds[i].revents & POLLIN`。
- `poll` 的第二个参数不是最大 fd 加 1，而是数组中要检查的元素数量，类型是 [[linux网络编程/概念词条/nfds_t|nfds_t]]。
- [[linux网络编程/函数笔记/IO多路复用/poll|poll]] 返回后仍然需要遍历数组，找出哪些元素的 `revents` 不为 0。

## 相关函数

- [[linux网络编程/函数笔记/IO多路复用/poll|poll]]
- [[linux网络编程/函数笔记/Socket/accept|accept]]
- [[linux网络编程/函数笔记/Socket/read|read]]
- [[linux网络编程/函数笔记/Socket/recv|recv]]

## 相关概念

- [[linux网络编程/概念词条/poll模型|poll模型]]
- [[linux网络编程/概念词条/poll事件宏|poll事件宏]]
- [[linux网络编程/概念词条/POLLRDNORM|POLLRDNORM]]
- [[linux网络编程/概念词条/nfds_t|nfds_t]]
- [[linux网络编程/概念词条/fd_set|fd_set]]
- [[linux网络编程/概念词条/事件就绪|事件就绪]]

## 相关课时

- [[linux网络编程/课时笔记/05 IO多路复用/02 poll|02 poll]]

## 相关模块

- [[linux网络编程/05 IO多路复用|05 IO多路复用]]
