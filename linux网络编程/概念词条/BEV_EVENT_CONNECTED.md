---
title: BEV_EVENT_CONNECTED
tags:
  - linux
  - 网络编程
  - 概念词条
  - 网络编程/Libevent
---
# BEV_EVENT_CONNECTED

`BEV_EVENT_CONNECTED` 是 [[linux网络编程/概念词条/bufferevent_event_cb|bufferevent_event_cb]] 中常见的事件标志，表示客户端主动连接已经成功建立。

它常出现在 [[linux网络编程/函数笔记/Libevent/bufferevent_socket_connect|bufferevent_socket_connect]] 之后的事件回调里。

## 判断方式

```c
if (events & BEV_EVENT_CONNECTED) {
    /* connect success */
}
```

## 相关函数

- [[linux网络编程/函数笔记/Libevent/bufferevent_socket_connect|bufferevent_socket_connect]]
- [[linux网络编程/函数笔记/Libevent/bufferevent_setcb|bufferevent_setcb]]

## 相关模块

- [[linux网络编程/07 Libevent库|07 Libevent库]]
