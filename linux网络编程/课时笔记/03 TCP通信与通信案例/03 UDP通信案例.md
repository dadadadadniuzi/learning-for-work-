---
title: UDP通信案例
tags:
  - linux
  - 网络编程
  - 课时笔记
  - 网络编程/TCP通信
  - 网络编程/UDP
---
# UDP通信案例

## 本节学什么

- [[linux网络编程/概念词条/UDP|UDP]] 的无连接通信特点
- [[linux网络编程/概念词条/UDP通信流程|UDP通信流程]]
- UDP 服务器端和客户端的调用链
- [[linux网络编程/函数笔记/Socket/sendto|sendto]] 和 [[linux网络编程/函数笔记/Socket/recvfrom|recvfrom]]
- UDP 数据报边界和客户端地址保存

## 本节学什么详解

- [[linux网络编程/概念词条/UDP|UDP]]：无连接、面向数据报，不需要 [[linux网络编程/函数笔记/Socket/listen|listen]]、[[linux网络编程/函数笔记/Socket/accept|accept]]，也不需要客户端先 [[linux网络编程/函数笔记/Socket/connect|connect]]。
- [[linux网络编程/概念词条/UDP通信流程|UDP通信流程]]：服务器通常 `socket -> bind -> recvfrom -> sendto`；客户端通常 `socket -> sendto -> recvfrom`。
- [[linux网络编程/函数笔记/Socket/sendto|sendto]]：发送 UDP 数据报时通常需要指定目标 IP 和端口。
- [[linux网络编程/函数笔记/Socket/recvfrom|recvfrom]]：接收 UDP 数据报时可以同时获得发送方地址，服务器回复时要用这个地址作为目标地址。
- UDP 数据报边界：一次 [[linux网络编程/函数笔记/Socket/sendto|sendto]] 发送一个数据报，接收端一次 [[linux网络编程/函数笔记/Socket/recvfrom|recvfrom]] 按数据报接收，不像 TCP 那样是连续字节流。

## 服务器端流程

```text
socket(AF_INET, SOCK_DGRAM, 0)
  ↓
bind(fd, 本地 IP + 端口)
  ↓
recvfrom(fd, buf, sizeof(buf), 0, 客户端地址, 地址长度)
  ↓
处理数据，例如小写转大写
  ↓
sendto(fd, buf, len, 0, 客户端地址, 地址长度)
```

服务器端需要 [[linux网络编程/函数笔记/Socket/bind|bind]] 固定端口，因为客户端必须知道要把数据发到哪里。

## 客户端流程

```text
socket(AF_INET, SOCK_DGRAM, 0)
  ↓
准备服务器地址
  ↓
sendto(fd, msg, len, 0, 服务器地址, 地址长度)
  ↓
recvfrom(fd, buf, sizeof(buf), 0, 可选来源地址, 地址长度)
```

UDP 客户端通常不需要 [[linux网络编程/函数笔记/Socket/bind|bind]]，内核会自动分配一个临时端口。当然，如果希望客户端使用固定端口，也可以主动 [[linux网络编程/函数笔记/Socket/bind|bind]]。

## 小写转大写案例思路

服务器端：

```c
int fd = socket(AF_INET, SOCK_DGRAM, 0);

struct sockaddr_in servaddr;
servaddr.sin_family = AF_INET;
servaddr.sin_port = htons(9527);
servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

bind(fd, (struct sockaddr *)&servaddr, sizeof(servaddr));

while (1) {
    char buf[1024];
    struct sockaddr_in cliaddr;
    socklen_t len = sizeof(cliaddr);

    ssize_t n = recvfrom(fd, buf, sizeof(buf), 0,
                         (struct sockaddr *)&cliaddr, &len);

    for (int i = 0; i < n; ++i) {
        buf[i] = toupper(buf[i]);
    }

    sendto(fd, buf, n, 0, (struct sockaddr *)&cliaddr, len);
}
```

客户端：

```c
int fd = socket(AF_INET, SOCK_DGRAM, 0);

struct sockaddr_in servaddr;
servaddr.sin_family = AF_INET;
servaddr.sin_port = htons(9527);
inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);

sendto(fd, "hello", 5, 0, (struct sockaddr *)&servaddr, sizeof(servaddr));

char buf[1024];
ssize_t n = recvfrom(fd, buf, sizeof(buf), 0, NULL, NULL);
```

## 和 TCP 案例的区别

| 对比项                 | TCP                                                                    | UDP                      |
| ------------------- | ---------------------------------------------------------------------- | ------------------------ |
| socket 类型           | `SOCK_STREAM`                                                          | `SOCK_DGRAM`             |
| 是否建立连接              | 需要三次握手                                                                 | 不需要连接                    |
| 服务端是否 listen/accept | 需要                                                                     | 不需要                      |
| 客户端是否 connect       | 通常需要                                                                   | 通常不需要                    |
| 常用收发函数              | [[send]]、/ [[recv]]<br>[[write]] 、[[linux网络编程/函数笔记/Socket/read\|read]] | [[sendto]]/ [[recvfrom]] |
| 数据边界                | 字节流，无天然边界                                                              | 数据报，有边界                  |
| 可靠性                 | 内核保证可靠、有序                                                              | 不保证可靠、有序、不重复             |

## 测试方法

可以使用 [[linux网络编程/指令查询/命令卡/nc|nc]] 测试 UDP 服务：

```bash
nc -u 127.0.0.1 9527
```

`-u` 表示使用 UDP。输入一行数据后，服务器如果实现了回显或大小写转换，就能看到回复。

## 易错点

- UDP 服务器不需要 [[linux网络编程/函数笔记/Socket/listen|listen]] 和 [[linux网络编程/函数笔记/Socket/accept|accept]]。
- UDP 客户端通常不需要 [[linux网络编程/函数笔记/Socket/connect|connect]]。
- 服务器回复客户端时，要使用 [[linux网络编程/函数笔记/Socket/recvfrom|recvfrom]] 得到的客户端地址，否则不知道该发给谁。
- `socklen_t len` 在调用 [[linux网络编程/函数笔记/Socket/recvfrom|recvfrom]] 前要初始化为地址结构大小。
- UDP 不保证可靠到达，测试时如果丢包或顺序变化，应用层要自己处理。

## 本节关键函数

- [[linux网络编程/函数笔记/Socket/socket|socket]]
- [[linux网络编程/函数笔记/Socket/bind|bind]]
- [[linux网络编程/函数笔记/Socket/sendto|sendto]]
- [[linux网络编程/函数笔记/Socket/recvfrom|recvfrom]]
- [[linux网络编程/函数笔记/地址转换/inet_pton|inet_pton]]
- [[linux网络编程/函数笔记/网络字节序/htons|htons]]
- [[linux网络编程/函数笔记/网络字节序/htonl|htonl]]

## 本节关键概念

- [[linux网络编程/概念词条/UDP|UDP]]
- [[linux网络编程/概念词条/UDP通信流程|UDP通信流程]]
- [[linux网络编程/概念词条/UDP数据报格式|UDP数据报格式]]
- [[linux网络编程/概念词条/套接字类型|套接字类型]]
- [[linux网络编程/概念词条/sockaddr_in|sockaddr_in]]
- [[linux网络编程/概念词条/socklen_t|socklen_t]]

## 关联模块

- [[linux网络编程/03 TCP通信与通信案例|03 TCP通信与通信案例]]
