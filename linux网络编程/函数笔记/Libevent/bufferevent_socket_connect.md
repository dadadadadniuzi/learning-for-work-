---
title: bufferevent_socket_connect
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/Libevent
---
# bufferevent_socket_connect

> [!info] 功能
> 让 [[linux网络编程/概念词条/bufferevent|bufferevent]] 主动连接服务器，常用于 Libevent 客户端。

## 函数原型

- `int bufferevent_socket_connect(struct bufferevent *bufev, struct sockaddr *addr, int socklen);`

## 依赖头文件

- `#include <event2/bufferevent.h>`
- `#include <sys/socket.h>`

## 输入参数

- `bufev`：目标 [[linux网络编程/概念词条/bufferevent|bufferevent]]。它可以由 [[linux网络编程/函数笔记/Libevent/bufferevent_socket_new|bufferevent_socket_new]] 创建，且 `fd` 参数可传 `-1`。
- `addr`：服务器地址，通常把 [[linux网络编程/概念词条/sockaddr_in|sockaddr_in]] 强制转换为 [[linux网络编程/概念词条/sockaddr|sockaddr]] 指针。
- `socklen`：地址结构长度，通常是 `sizeof(struct sockaddr_in)`。

## 输出参数

- 无直接输出参数。连接成功或失败会通过 [[linux网络编程/概念词条/bufferevent_event_cb|bufferevent_event_cb]] 通知。

## 返回值

- 成功启动连接过程返回 `0`。
- 失败返回 `-1`。

## 知识点补充

- 对非阻塞 socket 来说，返回 `0` 不代表连接已经完成，只代表连接动作已经发起。
- 真正连接成功通常在事件回调中检查 [[linux网络编程/概念词条/BEV_EVENT_CONNECTED|BEV_EVENT_CONNECTED]]。
- 连接失败、对端关闭和错误也会通过事件回调报告。

## 常见用法

```c
bufferevent_socket_connect(
    bev,
    (struct sockaddr *)&serv_addr,
    sizeof(serv_addr)
);
```

## 易错点

- 不要在返回 `0` 后立刻假设连接成功，应等待事件回调。
- `addr` 的地址族、端口字节序和 IP 字节序仍要按 socket 编程规则设置。

## 相关概念

- [[linux网络编程/概念词条/bufferevent|bufferevent]]
- [[linux网络编程/概念词条/Libevent连接流程|Libevent连接流程]]
- [[linux网络编程/概念词条/BEV_EVENT_CONNECTED|BEV_EVENT_CONNECTED]]
- [[linux网络编程/概念词条/sockaddr|sockaddr]]
- [[linux网络编程/概念词条/sockaddr_in|sockaddr_in]]

## 相关课时

- [[linux网络编程/课时笔记/07 Libevent库/05 bufferevent基础|05 bufferevent基础]]
- [[linux网络编程/课时笔记/07 Libevent库/06 evconnlistener与通信流程|06 evconnlistener与通信流程]]

## 相关模块

- [[linux网络编程/07 Libevent库|07 Libevent库]]
