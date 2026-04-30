---
title: socketpair
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/Socket
---
# socketpair

> [!info] 功能
> 创建一对已经互相连通的本地套接字，常用于同一台主机上的进程间通信。

## 函数原型

- `int socketpair(int domain, int type, int protocol, int sv[2]);`

## 依赖头文件

- `#include <sys/types.h>`
- `#include <sys/socket.h>`

## 输入参数

- `domain`
  地址族。
  `socketpair` 最常见也最经典的用法是传 `AF_UNIX` 或 `AF_LOCAL`，表示创建一对 [[linux网络编程/概念词条/Unix Domain Socket|Unix Domain Socket]]。

- `type`
  套接字类型。
  常见有：
  - `SOCK_STREAM`：流式、本质上像双向字节流
  - `SOCK_DGRAM`：数据报语义

- `protocol`
  具体协议。
  大多数场景直接传 `0`，表示使用默认协议。

- `sv`
  一个长度为 2 的整型数组，用来接收创建好的两个文件描述符。
  成功后：
  - `sv[0]` 和 `sv[1]` 就是一对互相连通的 socket
  - 往 `sv[0]` 写的数据，可以从 `sv[1]` 读到
  - 往 `sv[1]` 写的数据，也可以从 `sv[0]` 读到

## 输出参数

- `sv[0]`：一端的 socket 文件描述符
- `sv[1]`：另一端的 socket 文件描述符

## 返回值

- 成功返回 `0`
- 失败返回 `-1`，并设置 `errno`

## 怎么理解

`socketpair` 可以看成“直接帮你创建好一对已经连接完成的本地 socket”。

它和普通本地套接字最大的区别是：

- 不需要 `bind`
- 不需要 `listen`
- 不需要 `accept`
- 不需要 `connect`

创建出来后，两端马上就能通信。

## 常见用法

```c
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

int main(void) {
    int fd[2];
    char buf[128] = {0};

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd) == -1) {
        perror("socketpair");
        return 1;
    }

    write(fd[0], "hello", 5);
    read(fd[1], buf, sizeof(buf));

    printf("%s\n", buf);

    close(fd[0]);
    close(fd[1]);
    return 0;
}
```

## 和 `pipe` 的区别

- [[linux系统编程/函数笔记/IPC/pipe|pipe]] 默认更强调单向通信。
- `socketpair` 天然就是双向通信。
- `socketpair` 使用的是 socket 语义，更接近网络编程接口风格。
- `pipe` 更像传统管道；`socketpair` 更像“本机版已连接 socket”。

## 和普通 Unix Domain Socket 的区别

- 普通 Unix Domain Socket：
  通常要 `socket -> bind -> listen -> accept` 或 `socket -> connect`
- `socketpair`：
  一次调用直接得到一对已连接端点

所以 `socketpair` 更适合：

- 父子进程快速建立双向通信
- 线程/进程之间传控制消息
- 不想引入路径绑定时的本地通信

## 知识点补充

- `socketpair` 也是一种 [[linux网络编程/概念词条/本地通信|本地通信]] 方式。
- 它通常建立的是 [[linux网络编程/概念词条/Unix Domain Socket|Unix Domain Socket]]，不用于跨主机通信。
- 由于不依赖路径文件，所以不会遇到 `bind` 前 `unlink` 清理旧路径的问题。

## 易错点

- `sv` 必须是长度至少为 2 的整型数组。
- `socketpair` 成功后要记得关闭不用的一端，尤其在 `fork` 之后。
- 虽然它和 `pipe` 都能做 IPC，但语义和使用习惯并不完全一样。
- 它不是网络上的两端连接，而是本机内部的一对已连接 socket。

## 相关概念

- [[linux网络编程/概念词条/Unix Domain Socket|Unix Domain Socket]]
- [[linux网络编程/概念词条/AF_UNIX与AF_LOCAL|AF_UNIX与AF_LOCAL]]
- [[linux网络编程/概念词条/本地通信|本地通信]]

## 相关函数

- [[linux网络编程/函数笔记/Socket/socket|socket]]
- [[linux网络编程/函数笔记/Socket/bind|bind]]
- [[linux网络编程/函数笔记/Socket/connect|connect]]
- [[linux系统编程/函数笔记/IPC/pipe|pipe]]

## 相关课时

- [[linux网络编程/课时笔记/06 本地套接字与通信总结/01 Unix Domain Socket基础|01 Unix Domain Socket基础]]
- [[linux网络编程/课时笔记/06 本地套接字与通信总结/03 通信总结|03 通信总结]]

## 相关模块

- [[linux网络编程/06 本地套接字与通信总结|06 本地套接字与通信总结]]
