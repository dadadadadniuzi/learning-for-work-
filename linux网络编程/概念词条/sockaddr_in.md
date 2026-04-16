---
title: sockaddr_in
tags:
  - linux
  - 网络编程
  - 概念词条
---
# sockaddr_in

## 它是什么

- `struct sockaddr_in` 是 IPv4 专用的 socket 地址结构。
- 它保存 IPv4 通信所需的地址族、端口和 IP 地址。

## 常见原型

```c
struct sockaddr_in {
    sa_family_t    sin_family;
    in_port_t      sin_port;
    struct in_addr sin_addr;
};
```

## 字段说明

- `sin_family`：地址族，IPv4 通常填 `AF_INET`。
- `sin_port`：端口号，必须使用网络字节序，常写 `htons(port)`。
- `sin_addr`：IPv4 地址，类型是 [[linux网络编程/概念词条/in_addr|in_addr]]。

## 常见初始化

```c
struct sockaddr_in servaddr;
memset(&servaddr, 0, sizeof(servaddr));
servaddr.sin_family = AF_INET;
servaddr.sin_port = htons(9527);
servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
```

## 易错点

- `sin_port` 不要直接写普通整数端口。
- `sin_addr.s_addr` 不要直接写字符串 IP。
- 传给 `bind/connect` 时常需要转换为 `struct sockaddr *`。

## 常见出现位置

- [[linux网络编程/课时笔记/02 Socket编程基础/04 sockaddr与sockaddr_in结构|04 sockaddr与sockaddr_in结构]]
- [[linux网络编程/函数笔记/Socket/bind|bind]]
- [[linux网络编程/函数笔记/Socket/connect|connect]]
