---
title: SO_REUSEADDR
tags:
  - linux
  - 网络编程
  - 概念词条
---
# SO_REUSEADDR

## 它是什么

[[linux网络编程/概念词条/SO_REUSEADDR|SO_REUSEADDR]] 是 socket 层的地址复用选项，常通过 [[linux网络编程/函数笔记/Socket/setsockopt|setsockopt]] 设置。

它最常见的用途是：服务器退出后，本地端口因为 TCP 连接仍处于 [[linux网络编程/概念词条/TIME_WAIT|TIME_WAIT]] 等状态，导致马上重启服务器时 [[linux网络编程/函数笔记/Socket/bind|bind]] 报 `Address already in use`，这时可以用它缓解“端口短时间不可重新绑定”的问题。

## 依赖背景

- TCP 连接关闭后，主动关闭方通常会进入 [[linux网络编程/概念词条/TIME_WAIT|TIME_WAIT]]。
- [[linux网络编程/概念词条/TIME_WAIT|TIME_WAIT]] 会持续一段时间，通常和 [[linux网络编程/概念词条/MSL|MSL]] 有关。
- 如果服务器频繁重启，旧连接状态还没完全消失，新服务器立刻 [[linux网络编程/函数笔记/Socket/bind|bind]] 同一个 IP 和端口时可能失败。

## 常见写法

```c
int opt = 1;
setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
```

完整位置通常是：

```c
int lfd = socket(AF_INET, SOCK_STREAM, 0);

int opt = 1;
setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

bind(lfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
listen(lfd, 128);
```

## 参数含义

- `lfd`：监听 socket 的文件描述符。
- [[linux网络编程/概念词条/SOL_SOCKET|SOL_SOCKET]]：说明这是通用 socket 层选项。
- [[linux网络编程/概念词条/SO_REUSEADDR|SO_REUSEADDR]]：具体要开启的地址复用选项。
- `&opt`：选项值地址。`opt = 1` 表示开启，`opt = 0` 表示关闭。
- `sizeof(opt)`：选项值长度。

## 必须放在哪里

[[linux网络编程/概念词条/SO_REUSEADDR|SO_REUSEADDR]] 必须在 [[linux网络编程/函数笔记/Socket/bind|bind]] 之前设置。

原因是 [[linux网络编程/函数笔记/Socket/bind|bind]] 才是真正把 socket 和本地 IP、端口绑定起来的动作。等 [[linux网络编程/函数笔记/Socket/bind|bind]] 已经失败后再设置，已经来不及影响这次绑定结果。

## 和 SO_REUSEPORT 的区别

| 选项 | 主要作用 | 常见场景 |
|---|---|---|
| [[linux网络编程/概念词条/SO_REUSEADDR|SO_REUSEADDR]] | 允许地址复用，缓解端口处于等待状态导致的重新绑定失败 | 服务器快速重启、调试 |
| [[linux网络编程/概念词条/SO_REUSEPORT|SO_REUSEPORT]] | 允许多个 socket 绑定同一个 IP 和端口，并由内核分发连接或报文 | 多进程/多线程服务器负载分担 |

## 它不能解决什么

- 它不是“抢占端口”。如果另一个服务正在正常监听同一个端口，通常不能靠它强行抢过来。
- 它不能替代端口排查。要看谁占用了端口，应使用 [[linux网络编程/指令查询/命令卡/ss|ss]]、[[linux网络编程/指令查询/命令卡/netstat|netstat]]、[[linux网络编程/指令查询/命令卡/lsof|lsof]]。
- 它不能改变 TCP 的正常状态转换，只是让符合条件的地址复用更宽松。

## 怎么查询是否开启

可以用 [[linux网络编程/函数笔记/Socket/getsockopt|getsockopt]] 查询当前 socket 上该选项的值：

```c
int opt = 0;
socklen_t len = sizeof(opt);
getsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, &len);
```

## 常见出现位置

- [[linux网络编程/课时笔记/04 高并发服务器/03 端口复用|03 端口复用]]
- [[linux网络编程/函数笔记/Socket/setsockopt|setsockopt]]
- [[linux网络编程/函数笔记/Socket/getsockopt|getsockopt]]
- [[linux网络编程/函数笔记/Socket/bind|bind]]

## 相关概念

- [[linux网络编程/概念词条/端口复用|端口复用]]
- [[linux网络编程/概念词条/SOL_SOCKET|SOL_SOCKET]]
- [[linux网络编程/概念词条/SO_REUSEPORT|SO_REUSEPORT]]
- [[linux网络编程/概念词条/TIME_WAIT|TIME_WAIT]]
- [[linux网络编程/概念词条/MSL|MSL]]
