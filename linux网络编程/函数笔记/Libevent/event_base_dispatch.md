---
title: event_base_dispatch
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/Libevent
---
# event_base_dispatch

> [!info] 功能
> 启动 [[linux网络编程/概念词条/event_base|event_base]] 的事件循环，让 Libevent 开始等待并分发已经注册的事件。

## 函数原型

- `int event_base_dispatch(struct event_base *base);`

## 依赖头文件

- `#include <event2/event.h>`

## 输入参数

- `base`：由 [[linux网络编程/函数笔记/Libevent/event_base_new|event_base_new]] 创建的事件循环对象。它内部保存已注册事件、定时器和底层 IO 复用状态，不能传 `NULL`。

## 输出参数

- 无直接输出参数。事件发生时，Libevent 会自动调用对应的 [[linux网络编程/概念词条/事件回调函数|事件回调函数]]。

## 返回值

- 正常退出返回 `0`。
- 没有事件可处理时返回 `1`。
- 发生错误返回 `-1`。

## 知识点补充

- 这个函数相当于传统网络程序里的 `while + select/poll/epoll_wait` 主循环。
- 事件循环启动后，程序通常阻塞在这里，直到没有事件、主动退出或发生错误。
- 注册到 `base` 上的普通 [[linux网络编程/概念词条/event|event]]、[[linux网络编程/概念词条/bufferevent|bufferevent]]、[[linux网络编程/概念词条/evconnlistener|evconnlistener]] 都由它统一调度。

## 常见用法

```c
event_base_dispatch(base);
```

## 易错点

- 在调用它之前如果没有通过 [[linux网络编程/函数笔记/Libevent/event_add|event_add]] 或 listener/bufferevent 注册事件，循环可能直接结束。
- 回调函数不要长时间阻塞，否则会拖住整个事件循环。

## 相关概念

- [[linux网络编程/概念词条/event_base|event_base]]
- [[linux网络编程/概念词条/事件回调函数|事件回调函数]]
- [[linux网络编程/概念词条/事件的未决与非未决|事件的未决与非未决]]

## 相关课时

- [[linux网络编程/课时笔记/07 Libevent库/02 常规event基础|02 常规event基础]]

## 相关模块

- [[linux网络编程/07 Libevent库|07 Libevent库]]
