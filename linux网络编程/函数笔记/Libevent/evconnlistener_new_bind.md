---
title: evconnlistener_new_bind
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/Libevent
---
# evconnlistener_new_bind

> [!info] 功能
> 创建一个 [[linux网络编程/概念词条/evconnlistener|evconnlistener]]，完成服务器端监听 socket 的创建、绑定、监听，并在有客户端连接时触发回调。

## 函数原型

- `struct evconnlistener *evconnlistener_new_bind(struct event_base *base, evconnlistener_cb cb, void *ptr, unsigned flags, int backlog, const struct sockaddr *sa, int socklen);`

## 依赖头文件

- `#include <event2/listener.h>`
- `#include <sys/socket.h>`

## 输入参数

- `base`：所属的 [[linux网络编程/概念词条/event_base|event_base]]。
- `cb`：连接到来时调用的回调函数，类型是 [[linux网络编程/概念词条/evconnlistener_cb|evconnlistener_cb]]。回调中通常会拿到新的通信 socket，并创建 [[linux网络编程/概念词条/bufferevent|bufferevent]]。
- `ptr`：传给连接回调的自定义参数，可以传 `base`、配置结构体或 `NULL`。
- `flags`：listener 选项，常见有 [[linux网络编程/概念词条/LEV_OPT_CLOSE_ON_FREE与LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE 与 LEV_OPT_REUSEABLE]]。
- `backlog`：监听队列长度，含义接近 [[linux网络编程/函数笔记/Socket/listen|listen]] 的 `backlog`。
- `sa`：要绑定的地址，通常传 [[linux网络编程/概念词条/sockaddr_in|sockaddr_in]] 强制转换后的 [[linux网络编程/概念词条/sockaddr|sockaddr]] 指针。
- `socklen`：地址结构长度，例如 `sizeof(struct sockaddr_in)`。

## 输出参数

- 无直接输出参数。新连接会通过连接回调函数交给程序处理。

## 返回值

- 成功返回 `struct evconnlistener *`。
- 失败返回 `NULL`。

## 知识点补充

- 它把传统服务器的 [[linux网络编程/函数笔记/Socket/socket|socket]]、[[linux网络编程/函数笔记/Socket/bind|bind]]、[[linux网络编程/函数笔记/Socket/listen|listen]]、[[linux网络编程/函数笔记/Socket/accept|accept]] 流程封装起来。
- 回调拿到的 `fd` 是已连接套接字，后续通常交给 [[linux网络编程/函数笔记/Libevent/bufferevent_socket_new|bufferevent_socket_new]] 管理。
- 课程中这一节是从“手写 accept 循环”过渡到“Libevent 自动接收连接”的关键。

## 常见用法

```c
struct evconnlistener *listener = evconnlistener_new_bind(
    base,
    listen_cb,
    base,
    LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,
    10,
    (struct sockaddr *)&serv_addr,
    sizeof(serv_addr)
);
```

## 易错点

- `flags` 常写成按位或组合，不要写成逻辑或。
- 地址结构仍然要正确设置 `sin_family`、`sin_port`、`sin_addr`。
- 这个函数创建的是监听对象，不是单个客户端的通信对象。

## 相关概念

- [[linux网络编程/概念词条/evconnlistener|evconnlistener]]
- [[linux网络编程/概念词条/evconnlistener_cb|evconnlistener_cb]]
- [[linux网络编程/概念词条/LEV_OPT_CLOSE_ON_FREE与LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE 与 LEV_OPT_REUSEABLE]]
- [[linux网络编程/概念词条/监听套接字|监听套接字]]
- [[linux网络编程/概念词条/已连接套接字|已连接套接字]]

## 相关课时

- [[linux网络编程/课时笔记/07 Libevent库/06 evconnlistener与通信流程|06 evconnlistener与通信流程]]

## 相关模块

- [[linux网络编程/07 Libevent库|07 Libevent库]]
