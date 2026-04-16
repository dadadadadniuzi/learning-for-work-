---
title: event_new
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/Libevent
---
# event_new

> [!info] 功能
> 创建一个普通 [[linux网络编程/概念词条/event|event]]，把“监听哪个文件描述符、监听什么事件、触发后执行哪个回调”封装起来。

## 函数原型

- `struct event *event_new(struct event_base *base, evutil_socket_t fd, short what, event_callback_fn cb, void *arg);`

## 依赖头文件

- `#include <event2/event.h>`

## 输入参数

- `base`：事件所属的 [[linux网络编程/概念词条/event_base|event_base]]，由 [[linux网络编程/函数笔记/Libevent/event_base_new|event_base_new]] 创建。
- `fd`：要监听的文件描述符，类型是 [[linux网络编程/概念词条/evutil_socket_t|evutil_socket_t]]。可以是 socket、管道、标准输入等可被底层 IO 复用机制监听的描述符。
- `what`：监听的事件类型，常见取值来自 [[linux网络编程/概念词条/Libevent事件标志|Libevent事件标志]]，例如 `EV_READ`、`EV_WRITE`、`EV_PERSIST`。
- `cb`：事件触发时调用的回调函数，类型是 [[linux网络编程/概念词条/event_callback_fn|event_callback_fn]]。
- `arg`：传给回调函数的自定义参数，可以传结构体指针、`base` 指针或 `NULL`。

## 输出参数

- 无直接输出参数。创建出的事件对象通过返回值交给调用者。

## 返回值

- 成功返回 `struct event *`。
- 失败返回 `NULL`。

## 知识点补充

- `event_new` 只负责创建事件，还没有真正加入监听集合。
- 调用 [[linux网络编程/函数笔记/Libevent/event_add|event_add]] 后，事件才进入 [[linux网络编程/概念词条/事件的未决与非未决|未决]] 状态。
- 如果没有设置 `EV_PERSIST`，事件触发一次后会自动从未决集合移除。

## 常见用法

```c
struct event *ev = event_new(base, fd, EV_READ | EV_PERSIST, read_cb, NULL);
event_add(ev, NULL);
```

## 易错点

- 不要把局部变量地址作为 `arg` 传入后又离开其生命周期。
- `what` 里忘记写 `EV_PERSIST` 时，事件可能只触发一次。

## 相关概念

- [[linux网络编程/概念词条/event|event]]
- [[linux网络编程/概念词条/evutil_socket_t|evutil_socket_t]]
- [[linux网络编程/概念词条/event_callback_fn|event_callback_fn]]
- [[linux网络编程/概念词条/Libevent事件标志|Libevent事件标志]]

## 相关课时

- [[linux网络编程/课时笔记/07 Libevent库/02 常规event基础|02 常规event基础]]
- [[linux网络编程/课时笔记/07 Libevent库/03 event实现本地通信|03 event实现本地通信]]

## 相关模块

- [[linux网络编程/07 Libevent库|07 Libevent库]]
