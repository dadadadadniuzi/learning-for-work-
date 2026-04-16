---
title: evconnlistener_cb
tags:
  - linux
  - 网络编程
  - 概念词条
  - 网络编程/Libevent
---
# evconnlistener_cb

`evconnlistener_cb` 是 [[linux网络编程/概念词条/evconnlistener|evconnlistener]] 的连接回调类型。

## 函数形式

```c
void callback(
    struct evconnlistener *listener,
    evutil_socket_t fd,
    struct sockaddr *addr,
    int socklen,
    void *ctx
);
```

## 参数含义

- `listener`：触发连接事件的监听对象。
- `fd`：新客户端对应的已连接 socket，类型是 [[linux网络编程/概念词条/evutil_socket_t|evutil_socket_t]]。
- `addr`：客户端地址，通常可以按 [[linux网络编程/概念词条/sockaddr|sockaddr]] / [[linux网络编程/概念词条/sockaddr_in|sockaddr_in]] 理解。
- `socklen`：客户端地址结构长度。
- `ctx`：创建 listener 时传入的自定义参数。

## 在课程里怎么用

服务器通过 [[linux网络编程/函数笔记/Libevent/evconnlistener_new_bind|evconnlistener_new_bind]] 注册该回调。每当有客户端连接，回调中通常用 `fd` 创建 [[linux网络编程/函数笔记/Libevent/bufferevent_socket_new|bufferevent_socket_new]]。

## 相关模块

- [[linux网络编程/07 Libevent库|07 Libevent库]]
