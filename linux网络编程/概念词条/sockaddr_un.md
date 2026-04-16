---
title: sockaddr_un
tags:
  - linux
  - 网络编程
  - 概念词条
---
# sockaddr_un

## 它是什么

- `struct sockaddr_un` 是 Unix Domain Socket 使用的本地地址结构。
- 它在本地套接字中的地位类似 IPv4 网络套接字里的 [[linux网络编程/概念词条/sockaddr_in|sockaddr_in]]。

## 常见原型

```c
struct sockaddr_un {
    sa_family_t sun_family;
    char        sun_path[108];
};
```

## 字段说明

- `sun_family`：地址族，通常填 `AF_UNIX` 或 `AF_LOCAL`。
- `sun_path`：本地 socket 文件路径，用来标识通信端点。

## 怎么理解

- `sockaddr_in` 用 IP 和端口描述网络通信地址。
- `sockaddr_un` 用文件系统路径描述本机通信地址。
- 传给 `bind/connect` 时仍然需要转换为 `struct sockaddr *`。

## 常见写法

```c
struct sockaddr_un addr;
memset(&addr, 0, sizeof(addr));
addr.sun_family = AF_UNIX;
strcpy(addr.sun_path, "server.sock");
bind(fd, (struct sockaddr *)&addr, sizeof(addr));
```

## 易错点

- `sun_path` 是字符数组，不要写超出长度。
- `bind` 前旧路径存在时可能失败，服务端常先 `unlink(addr.sun_path)`。
- 这个路径是本地 socket 的名字，不是用来保存普通数据的文件。

## 常见出现位置

- [[linux网络编程/课时笔记/06 本地套接字与通信总结/02 sockaddr_un与路径绑定|02 sockaddr_un与路径绑定]]
- [[linux网络编程/概念词条/Unix Domain Socket|Unix Domain Socket]]
