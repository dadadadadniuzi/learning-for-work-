---
title: event_callback_fn
tags:
  - linux
  - 网络编程
  - 概念词条
  - 网络编程/Libevent
---
# event_callback_fn

`event_callback_fn` 是普通 [[linux网络编程/概念词条/event|event]] 的回调函数类型。

## 函数形式

```c
void callback(evutil_socket_t fd, short events, void *arg);
```

## 参数含义

- `fd`：触发事件的文件描述符，类型是 [[linux网络编程/概念词条/evutil_socket_t|evutil_socket_t]]。
- `events`：本次触发的事件类型，来自 [[linux网络编程/概念词条/Libevent事件标志|Libevent事件标志]]。
- `arg`：创建事件时传入的自定义参数。

## 在课程里怎么用

用 [[linux网络编程/函数笔记/Libevent/event_new|event_new]] 创建事件时，把该类型的函数作为 `cb` 参数传入。事件就绪后，[[linux网络编程/函数笔记/Libevent/event_base_dispatch|event_base_dispatch]] 会在事件循环中调用它。

## 相关概念

- [[linux网络编程/概念词条/事件回调函数|事件回调函数]]
- [[linux网络编程/概念词条/event|event]]

## 相关模块

- [[linux网络编程/07 Libevent库|07 Libevent库]]
