---
title: select
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/IO多路复用
---
# select

> [!info] 功能
> 同时监听多个文件描述符，等待其中一个或多个满足读、写或异常条件。

## 函数原型

- `int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);`

## 依赖头文件

- `#include <sys/select.h>`
- `#include <sys/time.h>`
- `#include <sys/types.h>`
- `#include <unistd.h>`

## 输入参数

- `nfds`：所有被监听 fd 中最大值加 1。内核只检查 `[0, nfds)` 范围内的 fd。
- `readfds`：读事件集合，传入时表示要监听哪些 fd 是否可读，返回时只保留已经可读的 fd。不关心可读事件可传 `NULL`。
- `writefds`：写事件集合，传入时表示要监听哪些 fd 是否可写，返回时只保留已经可写的 fd。不关心可写事件常传 `NULL`。
- `exceptfds`：异常事件集合，传入时表示要监听哪些 fd 是否有异常，基础 TCP 服务器中常传 `NULL`。
- `timeout`：超时时间。传 `NULL` 表示一直阻塞；传 `{0,0}` 表示立即返回；传正值表示最多等待指定时间。

## 输出参数

- `readfds`：返回后被修改，只保留可读 fd。
- `writefds`：返回后被修改，只保留可写 fd。
- `exceptfds`：返回后被修改，只保留异常 fd。
- `timeout`：某些系统上可能被修改，不建议循环复用同一个未重置的超时结构。

## 返回值

- 大于 `0`：就绪 fd 的总数量。
- 等于 `0`：超时，没有 fd 就绪。
- 等于 `-1`：出错。

## 知识点补充

- `select` 会修改传入的集合，所以通常维护 `allset`，每轮复制给 `rset`。
- 监听 fd 可读表示有新连接可 `accept`。
- 通信 fd 可读表示有数据可 `recv/read`，也可能是对端关闭。

## 常见用法

```c
fd_set rset = allset;
int nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
```

## 易错点

- `nfds` 必须是最大 fd 加 1，不是 fd 个数。
- 返回后要用 `FD_ISSET` 判断具体哪个 fd 就绪。
- 每轮循环不要直接复用被 `select` 改过的集合。

## 相关概念

- [[linux网络编程/概念词条/select模型|select模型]]
- [[linux网络编程/概念词条/fd_set|fd_set]]
- [[linux网络编程/概念词条/事件就绪|事件就绪]]

## 相关课时

- [[linux网络编程/课时笔记/05 IO多路复用/01 select|01 select]]

## 相关模块

- [[linux网络编程/05 IO多路复用|05 IO多路复用]]
