---
title: event_add
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/Libevent
---
# event_add

> [!info] 功能
> 把已经创建好的 [[linux网络编程/概念词条/event|event]] 加入事件循环，使它进入可被监听的未决状态。

## 函数原型

- `int event_add(struct event *ev, const struct timeval *timeout);`

## 依赖头文件

- `#include <event2/event.h>`
- `#include <sys/time.h>`

## 输入参数

- `ev`：由 [[linux网络编程/函数笔记/Libevent/event_new|event_new]] 创建的事件对象。
- `timeout`：超时时间，类型是 [[linux网络编程/概念词条/struct timeval|struct timeval]]。传 `NULL` 表示不设置超时，只等待 `fd` 上的事件；传具体时间表示事件也可以因为超时而触发。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回 `0`。
- 失败返回 `-1`。

## 知识点补充

- 调用后，事件从 [[linux网络编程/概念词条/事件的未决与非未决|非未决]] 变成 [[linux网络编程/概念词条/事件的未决与非未决|未决]]。
- 对非持久事件来说，触发一次后会自动变回非未决。
- 对带 `EV_PERSIST` 的事件来说，触发后仍然保持未决，可以继续监听。

## 常见用法

```c
event_add(ev, NULL);
```

## 易错点

- 只调用 [[linux网络编程/函数笔记/Libevent/event_new|event_new]] 不调用 `event_add`，事件不会被监听。
- 如果设置了超时参数，注意 `struct timeval` 变量在调用期间需要有效。

## 相关概念

- [[linux网络编程/概念词条/event|event]]
- [[linux网络编程/概念词条/事件的未决与非未决|事件的未决与非未决]]
- [[linux网络编程/概念词条/Libevent事件标志|Libevent事件标志]]
- [[linux网络编程/概念词条/struct timeval|struct timeval]]

## 相关课时

- [[linux网络编程/课时笔记/07 Libevent库/02 常规event基础|02 常规event基础]]
- [[linux网络编程/课时笔记/07 Libevent库/04 事件的未决与非未决|04 事件的未决与非未决]]

## 相关模块

- [[linux网络编程/07 Libevent库|07 Libevent库]]
