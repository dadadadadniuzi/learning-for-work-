---
title: setsockopt
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/Socket
---
# setsockopt

> [!info] 功能
> 设置 socket 选项，端口复用、缓冲区大小、超时等行为都可以通过它配置。

## 函数原型

- `int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);`

## 依赖头文件

- `#include <sys/types.h>`
- `#include <sys/socket.h>`

## 输入参数

- `sockfd`：要设置选项的 socket 文件描述符，通常由 [[linux网络编程/函数笔记/Socket/socket|socket]] 创建。设置端口复用时，一般在 `socket` 后、[[linux网络编程/函数笔记/Socket/bind|bind]] 前使用。
- `level`：选项所在协议层。通用 socket 层选项使用 [[linux网络编程/概念词条/SOL_SOCKET|SOL_SOCKET]]。
- `optname`：具体要设置的选项名称。端口复用常用 [[linux网络编程/概念词条/SO_REUSEADDR|SO_REUSEADDR]]，有些场景也会见到 [[linux网络编程/概念词条/SO_REUSEPORT|SO_REUSEPORT]]。
- `optval`：指向选项值的指针。设置布尔类选项时常用 `int opt = 1;`，再传 `&opt`。
- `optlen`：`optval` 指向数据的大小，类型是 [[linux网络编程/概念词条/socklen_t|socklen_t]]。传 `sizeof(opt)` 最常见。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回 `0`。
- 失败返回 `-1`，并设置错误信息。

## 知识点补充

- 端口复用不是 `bind` 的参数，而是通过 `setsockopt` 设置 socket 选项。
- 对服务器调试来说，最常见用途是避免程序重启后端口暂时无法绑定。
- 设置端口复用的位置很重要，通常必须在 `bind` 前。

## 常见用法

```c
int opt = 1;
setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
```

## 易错点

- `optval` 要传地址，不能直接传整数 `1`。
- `optlen` 要和 `optval` 指向的数据大小一致。
- 设置端口复用后仍可能因为其他活跃进程真正占用端口而绑定失败。

## 相关概念

- [[linux网络编程/概念词条/端口复用|端口复用]]
- [[linux网络编程/概念词条/SOL_SOCKET|SOL_SOCKET]]
- [[linux网络编程/概念词条/SO_REUSEADDR|SO_REUSEADDR]]
- [[linux网络编程/概念词条/SO_REUSEPORT|SO_REUSEPORT]]
- [[linux网络编程/概念词条/socklen_t|socklen_t]]

## 相关课时

- [[linux网络编程/课时笔记/04 高并发服务器/03 端口复用|03 端口复用]]

## 相关模块

- [[linux网络编程/04 高并发服务器|04 高并发服务器]]
