---
title: SO_REUSEPORT
tags:
  - linux
  - 网络编程
  - 概念词条
---
# SO_REUSEPORT

## 它是什么

[[linux网络编程/概念词条/SO_REUSEPORT|SO_REUSEPORT]] 是 socket 层的端口复用选项，允许多个 socket 绑定到相同的本地 IP 和端口。

在支持该选项的系统中，内核可以把新连接或数据报分配给这些 socket，常用于多进程或多线程服务器的负载分担。

## 常见写法

```c
int opt = 1;
setsockopt(lfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
```

有时也会和 [[linux网络编程/概念词条/SO_REUSEADDR|SO_REUSEADDR]] 一起设置：

```c
int opt = 1;
setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
setsockopt(lfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
```

## 输入含义

- `lfd`：监听 socket 或 UDP socket 的文件描述符。
- [[linux网络编程/概念词条/SOL_SOCKET|SOL_SOCKET]]：说明选项属于通用 socket 层。
- [[linux网络编程/概念词条/SO_REUSEPORT|SO_REUSEPORT]]：开启端口复用。
- `&opt`：选项值地址。`1` 表示开启，`0` 表示关闭。
- `sizeof(opt)`：选项值长度。

## 适用场景

- 多个进程都监听同一个端口，由内核分发连接。
- 避免单个监听进程成为瓶颈。
- UDP 服务希望多个 worker 共同接收同一端口上的数据报。

## 和 SO_REUSEADDR 的区别

| 选项               | 重点             | 课程中怎么记                     |                                  |
| ---------------- | -------------- | -------------------------- | -------------------------------- |
| [[SO_REUSEADDR]] | SO_REUSEADDR]] | 地址复用，重点缓解服务器重启后的绑定失败       | 常见于“端口被 TIME_WAIT 影响，重启 bind 失败” |
| [[SO_REUSEPORT]] | SO_REUSEPORT]] | 端口复用，重点允许多个 socket 绑定同一个端口 | 常见于“多个 worker 共同监听一个端口”          |

## 易错点

- [[linux网络编程/概念词条/SO_REUSEPORT|SO_REUSEPORT]] 不是所有旧系统都支持，Linux 中需要较新的内核版本。
- 多个 socket 想复用同一个端口时，通常每个 socket 都要在 [[linux网络编程/函数笔记/Socket/bind|bind]] 前设置该选项。
- 它不是单纯为了解决 [[linux网络编程/概念词条/TIME_WAIT|TIME_WAIT]] 的，解决服务器快速重启更常见的是 [[linux网络编程/概念词条/SO_REUSEADDR|SO_REUSEADDR]]。

## 怎么查询是否开启

可以通过 [[linux网络编程/函数笔记/Socket/getsockopt|getsockopt]] 查询当前 socket 的 [[linux网络编程/概念词条/SO_REUSEPORT|SO_REUSEPORT]] 值：

```c
int opt = 0;
socklen_t len = sizeof(opt);
getsockopt(lfd, SOL_SOCKET, SO_REUSEPORT, &opt, &len);
```

## 常见出现位置

- [[linux网络编程/课时笔记/04 高并发服务器/03 端口复用|03 端口复用]]
- [[linux网络编程/函数笔记/Socket/setsockopt|setsockopt]]
- [[linux网络编程/函数笔记/Socket/getsockopt|getsockopt]]

## 相关概念

- [[linux网络编程/概念词条/端口复用|端口复用]]
- [[linux网络编程/概念词条/SOL_SOCKET|SOL_SOCKET]]
- [[linux网络编程/概念词条/SO_REUSEADDR|SO_REUSEADDR]]
- [[linux网络编程/概念词条/多进程并发服务器|多进程并发服务器]]
- [[linux网络编程/概念词条/多线程并发服务器|多线程并发服务器]]
