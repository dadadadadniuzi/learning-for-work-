---
title: getsockopt
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/Socket
---
# getsockopt

> [!info] 功能
> 读取 socket 当前选项值。它和 [[linux网络编程/函数笔记/Socket/setsockopt|setsockopt]] 相反，一个负责查询，一个负责设置。

## 函数原型

- `int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen);`

## 依赖头文件

- `#include <sys/types.h>`
- `#include <sys/socket.h>`

## 输入参数

- `sockfd`：要查询选项的 socket 文件描述符。
- `level`：选项所在协议层。查询端口复用选项时通常使用 [[linux网络编程/概念词条/SOL_SOCKET|SOL_SOCKET]]。
- `optname`：要查询的选项名，例如 [[linux网络编程/概念词条/SO_REUSEADDR|SO_REUSEADDR]] 或 [[linux网络编程/概念词条/SO_REUSEPORT|SO_REUSEPORT]]。
- `optval`：保存选项值的缓冲区地址。对于布尔类选项，通常传 `int` 变量地址。
- `optlen`：输入时表示 `optval` 缓冲区大小；输出时被内核改写为实际返回的选项值长度，类型是 [[linux网络编程/概念词条/socklen_t|socklen_t]] 指针。

## 输出参数

- `optval`：保存查询到的选项值。
- `optlen`：保存实际返回的数据长度。

## 返回值

- 成功返回 `0`。
- 失败返回 `-1`，并设置 `errno`。

## 常见用法

```c
int opt = 0;
socklen_t len = sizeof(opt);

if (getsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, &len) == -1) {
    perror("getsockopt");
}

printf("SO_REUSEADDR = %d\n", opt);
```

## 在端口复用中的作用

端口复用主要通过 [[linux网络编程/函数笔记/Socket/setsockopt|setsockopt]] 设置。`getsockopt` 不是必须步骤，但可以用来确认某个 socket 当前是否开启了 [[linux网络编程/概念词条/SO_REUSEADDR|SO_REUSEADDR]] 或 [[linux网络编程/概念词条/SO_REUSEPORT|SO_REUSEPORT]]。

## 易错点

- `optlen` 是指针参数，要传 `&len`。
- 调用前要把 `len` 初始化为 `optval` 缓冲区大小。
- `getsockopt` 只能查询当前 socket 选项，不能直接告诉你系统里哪个进程占用了端口；查端口占用要用 [[linux网络编程/指令查询/命令卡/ss|ss]]、[[linux网络编程/指令查询/命令卡/netstat|netstat]] 或 [[linux网络编程/指令查询/命令卡/lsof|lsof]]。

## 相关概念

- [[linux网络编程/概念词条/端口复用|端口复用]]
- [[linux网络编程/概念词条/SOL_SOCKET|SOL_SOCKET]]
- [[linux网络编程/概念词条/SO_REUSEADDR|SO_REUSEADDR]]
- [[linux网络编程/概念词条/SO_REUSEPORT|SO_REUSEPORT]]
- [[linux网络编程/概念词条/socklen_t|socklen_t]]

## 相关函数

- [[linux网络编程/函数笔记/Socket/setsockopt|setsockopt]]
- [[linux网络编程/函数笔记/Socket/socket|socket]]
- [[linux网络编程/函数笔记/Socket/bind|bind]]

## 相关课时

- [[linux网络编程/课时笔记/04 高并发服务器/03 端口复用|03 端口复用]]

## 相关模块

- [[linux网络编程/04 高并发服务器|04 高并发服务器]]
