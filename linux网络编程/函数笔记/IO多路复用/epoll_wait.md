---
title: epoll_wait
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/IO多路复用
---
# epoll_wait

> [!info] 功能
> 等待 epoll 实例中已注册 fd 的就绪事件。

## 函数原型

- `int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);`

## 依赖头文件

- `#include <sys/epoll.h>`

## 输入参数

- `epfd`：`epoll_create` 返回的 epoll 文件描述符。
- `events`：接收就绪事件的数组首地址。
- `maxevents`：`events` 数组最多能容纳的事件数量，必须大于 0。
- `timeout`：超时时间，单位毫秒。`-1` 表示一直阻塞，`0` 表示立即返回，正数表示最多等待指定毫秒数。

## 输出参数

- `events`：成功返回时，前若干个元素保存已经就绪的事件。

## 返回值

- 大于 `0`：就绪事件数量。
- 等于 `0`：超时。
- 等于 `-1`：出错。

## 知识点补充

- 返回值 `n` 表示 `events[0]` 到 `events[n-1]` 有效。
- 常通过 `events[i].data.fd` 判断哪个 fd 就绪。

## 常见用法

```c
struct epoll_event events[1024];
int nready = epoll_wait(epfd, events, 1024, -1);
```

## 易错点

- `maxevents` 不能超过数组实际容量。
- `epoll_wait` 返回的是就绪事件，不是所有已注册 fd。

## 相关概念

- [[linux网络编程/概念词条/epoll模型|epoll模型]]
- [[linux网络编程/概念词条/epoll_event|epoll_event]]
- [[linux网络编程/概念词条/事件就绪|事件就绪]]

## 相关课时

- [[linux网络编程/课时笔记/05 IO多路复用/03 epoll|03 epoll]]

## 相关模块

- [[linux网络编程/05 IO多路复用|05 IO多路复用]]
