---
title: BEV_EVENT_EOF
tags:
  - linux
  - 网络编程
  - 概念词条
  - 网络编程/Libevent
---
# BEV_EVENT_EOF

`BEV_EVENT_EOF` 表示 bufferevent 检测到对端关闭连接。

在 TCP 通信里，这通常对应传统 `recv` 返回 `0` 的场景：对端正常关闭，当前连接应该做资源清理。

## 常见处理

```c
if (events & BEV_EVENT_EOF) {
    bufferevent_free(bev);
}
```

## 相关函数

- [[linux网络编程/函数笔记/Libevent/bufferevent_setcb|bufferevent_setcb]]
- [[linux网络编程/函数笔记/Libevent/bufferevent_free|bufferevent_free]]

## 相关模块

- [[linux网络编程/07 Libevent库|07 Libevent库]]
