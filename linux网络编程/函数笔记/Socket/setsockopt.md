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
> 设置 socket 选项。网络编程中最常见的用法是开启 [[linux网络编程/概念词条/端口复用|端口复用]]，例如设置 [[linux网络编程/概念词条/SO_REUSEADDR|SO_REUSEADDR]] 或 [[linux网络编程/概念词条/SO_REUSEPORT|SO_REUSEPORT]]；也可以设置 [[linux网络编程/概念词条/SO_LINGER|SO_LINGER]] 控制关闭连接时的行为。

## 函数原型

- `int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);`

## 依赖头文件

- `#include <sys/types.h>`
- `#include <sys/socket.h>`

## 输入参数

- `sockfd`：要设置选项的 socket 文件描述符，通常由 [[linux网络编程/函数笔记/Socket/socket|socket]] 创建。设置端口复用时，一般在 `socket` 后、[[linux网络编程/函数笔记/Socket/bind|bind]] 前使用。
- `level`：选项所在协议层。端口复用属于通用 socket 层选项，通常传 [[linux网络编程/概念词条/SOL_SOCKET|SOL_SOCKET]]。
- `optname`：具体选项名称。端口复用常用 [[linux网络编程/概念词条/SO_REUSEADDR|SO_REUSEADDR]]；多进程负载分担场景可能使用 [[linux网络编程/概念词条/SO_REUSEPORT|SO_REUSEPORT]]。
- `optval`：指向选项值的指针。布尔类选项通常定义 `int opt = 1;` 表示开启，再传 `&opt`；如果要关闭，可传值为 `0` 的整数地址。像 [[linux网络编程/概念词条/SO_LINGER|SO_LINGER]] 这类选项，则需要传 [[linux网络编程/概念词条/struct linger|struct linger]] 的地址。
- `optlen`：`optval` 指向数据的大小，类型是 [[linux网络编程/概念词条/socklen_t|socklen_t]]。布尔类 `int` 选项常传 `sizeof(opt)`。

## 输出参数

- 无直接输出参数。函数通过返回值表示是否设置成功。

## 返回值

- 成功返回 `0`。
- 失败返回 `-1`，并设置 `errno`。

## 端口复用典型用法

```c
int lfd = socket(AF_INET, SOCK_STREAM, 0);

int opt = 1;
int ret = setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
if (ret == -1) {
    perror("setsockopt");
}

bind(lfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
```

## 为什么要在 bind 前调用

[[linux网络编程/函数笔记/Socket/bind|bind]] 会执行地址和端口绑定检查。如果绑定检查之前没有告诉内核“允许复用”，那么这次 `bind` 仍可能因为地址占用失败。

因此端口复用的顺序通常是：

```text
socket -> setsockopt -> bind -> listen
```

## 常见端口复用选项

- [[linux网络编程/概念词条/SO_REUSEADDR|SO_REUSEADDR]]：常用于服务重启后快速重新绑定端口。
- [[linux网络编程/概念词条/SO_REUSEPORT|SO_REUSEPORT]]：允许多个 socket 绑定同一 IP 和端口，适合多进程负载分担。

## 和 getsockopt 的关系

- `setsockopt`：设置选项。
- [[linux网络编程/函数笔记/Socket/getsockopt|getsockopt]]：读取选项。

调试时如果怀疑选项没生效，可以用 `getsockopt` 查询。

## 易错点

- `optval` 要传地址，不能直接传整数 `1`。
- `optlen` 要和 `optval` 指向的数据大小一致。
- 必须检查返回值，否则设置失败了也可能继续执行到 `bind`。
- 设置端口复用后仍可能因为其他活跃进程真正占用端口而绑定失败。

## 相关概念

- [[linux网络编程/概念词条/端口复用|端口复用]]
- [[linux网络编程/概念词条/SOL_SOCKET|SOL_SOCKET]]
- [[linux网络编程/概念词条/SO_REUSEADDR|SO_REUSEADDR]]
- [[linux网络编程/概念词条/SO_REUSEPORT|SO_REUSEPORT]]
- [[linux网络编程/概念词条/SO_LINGER|SO_LINGER]]
- [[linux网络编程/概念词条/struct linger|struct linger]]
- [[linux网络编程/概念词条/socklen_t|socklen_t]]
- [[linux网络编程/概念词条/TIME_WAIT|TIME_WAIT]]

## 相关函数

- [[linux网络编程/函数笔记/Socket/getsockopt|getsockopt]]
- [[linux网络编程/函数笔记/Socket/socket|socket]]
- [[linux网络编程/函数笔记/Socket/bind|bind]]
- [[linux网络编程/函数笔记/Socket/listen|listen]]

## 相关课时

- [[linux网络编程/课时笔记/04 高并发服务器/03 端口复用|03 端口复用]]

## 相关模块

- [[linux网络编程/04 高并发服务器|04 高并发服务器]]
