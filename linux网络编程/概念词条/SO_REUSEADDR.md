---
title: SO_REUSEADDR
tags:
  - linux
  - 网络编程
  - 概念词条
---
# SO_REUSEADDR

## 它是什么

- `SO_REUSEADDR` 是常见 socket 选项，用于允许地址复用。
- TCP 服务器调试时经常用它避免服务重启后短时间内无法重新绑定端口。

## 常见写法

```c
int opt = 1;
setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
```

## 怎么理解

- 它告诉内核：这个 socket 允许复用本地地址相关资源。
- 它通常解决“服务刚退出，端口短时间内仍不可绑定”的问题。

## 易错点

- 需要在 `bind` 前设置。
- 它不是“强制抢占其他正在监听的服务端口”。

## 常见出现位置

- [[linux网络编程/课时笔记/04 高并发服务器/03 端口复用|03 端口复用]]
- [[linux网络编程/函数笔记/Socket/setsockopt|setsockopt]]
