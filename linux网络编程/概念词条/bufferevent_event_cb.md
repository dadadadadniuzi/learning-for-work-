---
title: bufferevent_event_cb
tags:
  - linux
  - 网络编程
  - 概念词条
  - 网络编程/Libevent
---
# bufferevent_event_cb

`bufferevent_event_cb` 是 [[linux网络编程/概念词条/bufferevent|bufferevent]] 的事件回调类型，用来处理连接成功、连接失败、对端关闭和错误。

## 函数形式

```c
void callback(struct bufferevent *bev, short events, void *ctx);
```

## 参数含义

- `bev`：触发事件的 bufferevent。
- `events`：事件标志，常见有 [[linux网络编程/概念词条/BEV_EVENT_CONNECTED|BEV_EVENT_CONNECTED]]、[[linux网络编程/概念词条/BEV_EVENT_EOF|BEV_EVENT_EOF]]、[[linux网络编程/概念词条/BEV_EVENT_ERROR|BEV_EVENT_ERROR]]。
- `ctx`：调用 [[linux网络编程/函数笔记/Libevent/bufferevent_setcb|bufferevent_setcb]] 时传入的自定义参数。

## 在课程里怎么用

客户端主动连接时，连接结果不是直接从 [[linux网络编程/函数笔记/Libevent/bufferevent_socket_connect|bufferevent_socket_connect]] 得到，而是在这个回调里根据事件标志判断。

## 相关概念

- [[linux网络编程/概念词条/bufferevent回调|bufferevent回调]]
- [[linux网络编程/概念词条/Libevent连接流程|Libevent连接流程]]

## 相关模块

- [[linux网络编程/07 Libevent库|07 Libevent库]]
