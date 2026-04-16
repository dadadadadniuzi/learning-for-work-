---
title: bufferevent_socket_new
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/Libevent
---
# bufferevent_socket_new

> [!info] 功能
> 基于 socket 创建一个 [[linux网络编程/概念词条/bufferevent|bufferevent]]，让 Libevent 帮我们管理读缓冲区、写缓冲区和读写事件。

## 函数原型

- `struct bufferevent *bufferevent_socket_new(struct event_base *base, evutil_socket_t fd, enum bufferevent_options options);`

## 依赖头文件

- `#include <event2/bufferevent.h>`

## 输入参数

- `base`：所属的 [[linux网络编程/概念词条/event_base|event_base]]。
- `fd`：通信 socket，类型是 [[linux网络编程/概念词条/evutil_socket_t|evutil_socket_t]]。如果已经有连接好的 socket，就传该描述符；如果后面准备用 [[linux网络编程/函数笔记/Libevent/bufferevent_socket_connect|bufferevent_socket_connect]] 主动连接，可以传 `-1`。
- `options`：bufferevent 选项，常见值是 [[linux网络编程/概念词条/BEV_OPT_CLOSE_ON_FREE|BEV_OPT_CLOSE_ON_FREE]]，表示释放 bufferevent 时自动关闭底层 socket。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回 `struct bufferevent *`。
- 失败返回 `NULL`。

## 知识点补充

- [[linux网络编程/概念词条/bufferevent|bufferevent]] 是对普通事件的高级封装，重点解决“socket 读写缓冲管理”。
- 创建后通常要调用 [[linux网络编程/函数笔记/Libevent/bufferevent_setcb|bufferevent_setcb]] 设置回调，再调用 [[linux网络编程/函数笔记/Libevent/bufferevent_enable|bufferevent_enable]] 启用读写事件。
- 它内部包含 [[linux网络编程/概念词条/bufferevent读写缓冲区|bufferevent读写缓冲区]]，因此读写一般使用 [[linux网络编程/函数笔记/Libevent/bufferevent_read|bufferevent_read]] 和 [[linux网络编程/函数笔记/Libevent/bufferevent_write|bufferevent_write]]。

## 常见用法

```c
struct bufferevent *bev = bufferevent_socket_new(
    base, fd, BEV_OPT_CLOSE_ON_FREE
);
```

## 易错点

- 只创建 bufferevent 不会自动调用读回调，必须启用 `EV_READ`。
- 如果没有传 [[linux网络编程/概念词条/BEV_OPT_CLOSE_ON_FREE|BEV_OPT_CLOSE_ON_FREE]]，释放 bufferevent 后底层 socket 可能仍需要手动关闭。

## 相关概念

- [[linux网络编程/概念词条/bufferevent|bufferevent]]
- [[linux网络编程/概念词条/evutil_socket_t|evutil_socket_t]]
- [[linux网络编程/概念词条/BEV_OPT_CLOSE_ON_FREE|BEV_OPT_CLOSE_ON_FREE]]

## 相关课时

- [[linux网络编程/课时笔记/07 Libevent库/05 bufferevent基础|05 bufferevent基础]]

## 相关模块

- [[linux网络编程/07 Libevent库|07 Libevent库]]
