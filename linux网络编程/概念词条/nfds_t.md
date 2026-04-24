---
title: nfds_t
tags:
  - linux
  - 网络编程
  - 概念词条
  - 网络编程/IO多路复用
---
# nfds_t

## 它是什么

[[linux网络编程/概念词条/nfds_t|nfds_t]] 是 [[linux网络编程/函数笔记/IO多路复用/poll|poll]] 函数第二个参数 `nfds` 的类型，用来表示 `struct pollfd` 数组中要检查的元素数量。

## 依赖头文件

```c
#include <poll.h>
```

## 在 poll 原型中的位置

```c
int poll(struct pollfd *fds, nfds_t nfds, int timeout);
```

- `fds`：[[linux网络编程/概念词条/pollfd|struct pollfd]] 数组首地址。
- `nfds`：要让内核检查的数组元素个数，类型就是 [[linux网络编程/概念词条/nfds_t|nfds_t]]。
- `timeout`：超时时间，单位是毫秒。

## 怎么理解

[[linux网络编程/概念词条/nfds_t|nfds_t]] 通常可以理解为一个无符号整数类型。它不是文件描述符类型，而是“fd 数组元素数量”的类型。

例如：

```c
struct pollfd fds[1024];
nfds_t nfds = max_index + 1;

poll(fds, nfds, -1);
```

这里的 `nfds` 表示 `fds[0]` 到 `fds[nfds - 1]` 这些元素会被 [[linux网络编程/函数笔记/IO多路复用/poll|poll]] 检查。

## 和 select 的 nfds 区别

| 函数         | 参数含义              |            |                             |
| ---------- | ----------------- | ---------- | --------------------------- |
| [[select]] | select]] 的 `nfds` | 最大 fd 加 1  |                             |
| [[poll]]   | poll]] 的 `nfds`   | [[pollfd]] | struct pollfd]] 数组中要检查的元素数量 |

这点很容易混淆：[[linux网络编程/函数笔记/IO多路复用/poll|poll]] 不关心最大 fd 是多少，它只按数组长度扫描。

## 常见写法

如果用 `max_index` 记录当前数组中使用到的最大下标：

```c
int max_index = 0;
struct pollfd fds[1024];

fds[0].fd = lfd;
fds[0].events = POLLIN;

poll(fds, (nfds_t)(max_index + 1), -1);
```

如果数组前 `n` 个元素都是有效监听项：

```c
nfds_t n = 10;
poll(fds, n, 3000);
```

## 易错点

- [[linux网络编程/概念词条/nfds_t|nfds_t]] 不是最大 fd 加 1，这一点和 [[linux网络编程/函数笔记/IO多路复用/select|select]] 不同。
- `nfds` 太小会导致数组后面的 fd 根本不被检查。
- `nfds` 太大则会让 [[linux网络编程/函数笔记/IO多路复用/poll|poll]] 扫描无意义的数组元素，所以不用的位置通常把 `fd` 设为 `-1`。
- 它不是 `size_t`，但含义上也属于“数量类型”，学习时可以先按“数组元素个数”理解。

## 相关函数

- [[linux网络编程/函数笔记/IO多路复用/poll|poll]]

## 相关概念

- [[linux网络编程/概念词条/pollfd|struct pollfd]]
- [[linux网络编程/概念词条/poll模型|poll模型]]
- [[linux网络编程/概念词条/poll事件宏|poll事件宏]]
- [[linux网络编程/概念词条/fd_set|fd_set]]

## 相关课时

- [[linux网络编程/课时笔记/05 IO多路复用/02 poll|02 poll]]

## 相关模块

- [[linux网络编程/05 IO多路复用|05 IO多路复用]]
