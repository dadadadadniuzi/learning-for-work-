---
title: poll
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/IO多路复用
---
# poll

> [!info] 功能
> 使用 `pollfd` 数组监听多个 fd 的事件就绪状态。

## 函数原型

- `int poll(struct pollfd *fds, nfds_t nfds, int timeout);`

## 依赖头文件

- `#include <poll.h>`

## 输入参数

- `fds`：`pollfd` 数组首地址。数组中每个元素描述一个要监听的 fd、关心事件和返回事件。
- `nfds`：数组中有效元素数量，类型是 `nfds_t`。
- `timeout`：超时时间，单位毫秒。`-1` 表示一直阻塞，`0` 表示立即返回，正数表示最多等待指定毫秒数。

## 输出参数

- `fds[i].revents`：内核写入的实际就绪事件。调用前通常不需要手动设置，但调用后要检查它。

## 返回值

- 大于 `0`：就绪 fd 的数量。
- 等于 `0`：超时。
- 等于 `-1`：出错。

## 知识点补充

- `poll` 用数组替代 `fd_set`。
- `events` 是用户设置的关注事件，`revents` 是内核返回的实际事件。
- 常见读事件是 `POLLIN`。

## 常见用法

```c
struct pollfd fds[1024];
fds[0].fd = lfd;
fds[0].events = POLLIN;
int nready = poll(fds, max_index + 1, -1);
```

## 易错点

- `nfds` 是数组元素数量，不是最大 fd 加 1。
- 返回后仍然需要遍历数组检查 `revents`。
- 不用的元素可以把 `fd` 设为 `-1`。

## 相关概念

- [[linux网络编程/概念词条/poll模型|poll模型]]
- [[linux网络编程/概念词条/pollfd|pollfd]]
- [[linux网络编程/概念词条/事件就绪|事件就绪]]

## 相关课时

- [[linux网络编程/课时笔记/05 IO多路复用/02 poll|02 poll]]

## 相关模块

- [[linux网络编程/05 IO多路复用|05 IO多路复用]]
