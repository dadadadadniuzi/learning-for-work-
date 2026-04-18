---
title: SOL_SOCKET
tags:
  - linux
  - 网络编程
  - 概念词条
---
# SOL_SOCKET

## 它是什么

[[linux网络编程/概念词条/SOL_SOCKET|SOL_SOCKET]] 是 [[linux网络编程/函数笔记/Socket/setsockopt|setsockopt]] 和 [[linux网络编程/函数笔记/Socket/getsockopt|getsockopt]] 的 `level` 参数常用取值，表示“操作的是通用 socket 层选项”。

它不是某个具体功能开关，而是告诉内核：后面的 `optname` 要到 socket 层去解释。

## 为什么需要 level

socket 选项分布在不同协议层。例如：

| level | 含义 | 常见选项 |
|---|---|---|
| [[linux网络编程/概念词条/SOL_SOCKET|SOL_SOCKET]] | 通用 socket 层 | [[linux网络编程/概念词条/SO_REUSEADDR|SO_REUSEADDR]]、[[linux网络编程/概念词条/SO_REUSEPORT|SO_REUSEPORT]] |
| `IPPROTO_TCP` | TCP 协议层 | `TCP_NODELAY` |
| `IPPROTO_IP` | IPv4 协议层 | `IP_TTL` |

所以调用 [[linux网络编程/函数笔记/Socket/setsockopt|setsockopt]] 时，`level` 和 `optname` 要配套使用。

## 常见搭配

```c
int opt = 1;
setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
```

这句可以拆开看：

- [[linux网络编程/概念词条/SOL_SOCKET|SOL_SOCKET]]：到 socket 层找选项。
- [[linux网络编程/概念词条/SO_REUSEADDR|SO_REUSEADDR]]：具体选项是地址复用。
- `&opt`：开启该选项。

## 在端口复用中的作用

[[linux网络编程/概念词条/端口复用|端口复用]]相关的 [[linux网络编程/概念词条/SO_REUSEADDR|SO_REUSEADDR]]、[[linux网络编程/概念词条/SO_REUSEPORT|SO_REUSEPORT]] 都属于 socket 层选项，所以 `level` 通常写 [[linux网络编程/概念词条/SOL_SOCKET|SOL_SOCKET]]。

## 易错点

- [[linux网络编程/概念词条/SOL_SOCKET|SOL_SOCKET]] 不是“开启复用”的选项，真正的开关是 [[linux网络编程/概念词条/SO_REUSEADDR|SO_REUSEADDR]] 或 [[linux网络编程/概念词条/SO_REUSEPORT|SO_REUSEPORT]]。
- `level` 写错时，内核会按错误的协议层解释 `optname`，可能导致 [[linux网络编程/函数笔记/Socket/setsockopt|setsockopt]] 失败。
- [[linux网络编程/概念词条/SOL_SOCKET|SOL_SOCKET]] 常用于通用 socket 行为，不专属于 TCP。

## 常见出现位置

- [[linux网络编程/函数笔记/Socket/setsockopt|setsockopt]]
- [[linux网络编程/函数笔记/Socket/getsockopt|getsockopt]]
- [[linux网络编程/课时笔记/04 高并发服务器/03 端口复用|03 端口复用]]

## 相关概念

- [[linux网络编程/概念词条/端口复用|端口复用]]
- [[linux网络编程/概念词条/SO_REUSEADDR|SO_REUSEADDR]]
- [[linux网络编程/概念词条/SO_REUSEPORT|SO_REUSEPORT]]
