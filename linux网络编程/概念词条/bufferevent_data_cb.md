---
title: bufferevent_data_cb
tags:
  - linux
  - 网络编程
  - 概念词条
  - 网络编程/Libevent
---
# bufferevent_data_cb

`bufferevent_data_cb` 是 [[linux网络编程/概念词条/bufferevent|bufferevent]] 的读回调和写回调类型。

## 函数形式

```c
void callback(struct bufferevent *bev, void *ctx);
```

## 参数含义

- `bev`：触发回调的 [[linux网络编程/概念词条/bufferevent|bufferevent]]。
- `ctx`：调用 [[linux网络编程/函数笔记/Libevent/bufferevent_setcb|bufferevent_setcb]] 时传入的自定义参数。

## 在课程里怎么用

- 读回调里通常调用 [[linux网络编程/函数笔记/Libevent/bufferevent_read|bufferevent_read]] 取出数据。
- 写回调用于关注输出缓冲区状态，入门阶段经常传 `NULL`。

## 相关概念

- [[linux网络编程/概念词条/bufferevent回调|bufferevent回调]]
- [[linux网络编程/概念词条/bufferevent读写缓冲区|bufferevent读写缓冲区]]

## 相关模块

- [[linux网络编程/07 Libevent库|07 Libevent库]]
