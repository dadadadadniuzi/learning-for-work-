---
title: sun_path
tags:
  - linux
  - 网络编程
  - 概念词条
---
# sun_path

## 它是什么

- `sun_path` 是 [[linux网络编程/概念词条/sockaddr_un|sockaddr_un]] 结构中的路径字段。
- 它保存 Unix Domain Socket 的本地路径名。

## 怎么理解

- 网络 socket 使用 IP 和端口找到服务。
- Unix Domain Socket 常使用 `sun_path` 里的路径找到服务。
- 这个路径对应的是 socket 文件名，用于命名通信端点。

## 常见写法

```c
strcpy(addr.sun_path, "server.sock");
```

## 易错点

- `sun_path` 长度有限，不能写过长路径。
- 服务端绑定前旧路径存在可能导致 `bind` 失败。
- 程序退出后通常要清理该路径。

## 常见出现位置

- [[linux网络编程/概念词条/sockaddr_un|sockaddr_un]]
- [[linux网络编程/课时笔记/06 本地套接字与通信总结/02 sockaddr_un与路径绑定|02 sockaddr_un与路径绑定]]
