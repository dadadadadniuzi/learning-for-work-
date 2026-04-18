---
title: FD_SETSIZE
tags:
  - linux
  - 网络编程
  - 概念词条
  - 网络编程/IO多路复用
---
# FD_SETSIZE

## 它是什么

[[linux网络编程/概念词条/FD_SETSIZE|FD_SETSIZE]] 是和 [[linux网络编程/概念词条/fd_set|fd_set]] 相关的容量限制宏，表示 [[linux网络编程/概念词条/fd_set|fd_set]] 默认最多能表示多少个文件描述符。

在很多系统中，默认值常见为 `1024`，但具体值以系统头文件和编译环境为准。

## 为什么它重要

[[linux网络编程/函数笔记/IO多路复用/select|select]] 使用 [[linux网络编程/概念词条/fd_set|fd_set]] 位图管理 fd。位图大小有限，因此 [[linux网络编程/概念词条/select模型|select模型]] 不能天然支持无限数量的 fd。

当服务器连接数很多时，[[linux网络编程/概念词条/FD_SETSIZE|FD_SETSIZE]] 会成为 [[linux网络编程/概念词条/select模型|select模型]] 的一个限制。

## 和 nfds 的区别

| 名称 | 含义 |
|---|---|
| [[linux网络编程/概念词条/FD_SETSIZE|FD_SETSIZE]] | [[linux网络编程/概念词条/fd_set|fd_set]] 能表示的容量上限 |
| `nfds` | 调用 [[linux网络编程/函数笔记/IO多路复用/select|select]] 时告诉内核要检查 `[0, nfds)` 范围内的 fd，通常是最大 fd 加 1 |

简单说：[[linux网络编程/概念词条/FD_SETSIZE|FD_SETSIZE]] 是集合容量，`nfds` 是本次检查范围。

## 怎么查看

可以写一个小程序打印：

```c
#include <stdio.h>
#include <sys/select.h>

int main(void) {
    printf("%d\n", FD_SETSIZE);
    return 0;
}
```

也可以在系统头文件中搜索 `FD_SETSIZE`，但不同系统和 C 库定义位置可能不同。

## 易错点

- 不要把 [[linux网络编程/概念词条/FD_SETSIZE|FD_SETSIZE]] 理解成“当前进程最多能打开的 fd 数”。进程 fd 上限还和 `ulimit -n` 等资源限制有关。
- 即使进程能打开很多 fd，[[linux网络编程/概念词条/fd_set|fd_set]] 也可能因为 [[linux网络编程/概念词条/FD_SETSIZE|FD_SETSIZE]] 受限。
- 如果要处理大量连接，实际工程中更常使用 [[linux网络编程/概念词条/epoll模型|epoll模型]]。

## 相关概念

- [[linux网络编程/概念词条/fd_set|fd_set]]
- [[linux网络编程/概念词条/select模型|select模型]]
- [[linux网络编程/概念词条/poll模型|poll模型]]
- [[linux网络编程/概念词条/epoll模型|epoll模型]]

## 相关函数

- [[linux网络编程/函数笔记/IO多路复用/select|select]]

## 相关课时

- [[linux网络编程/课时笔记/05 IO多路复用/01 select|01 select]]
