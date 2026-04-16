---
title: bufferevent_enable
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/Libevent
---
# bufferevent_enable

> [!info] 功能
> 启用 [[linux网络编程/概念词条/bufferevent|bufferevent]] 的读事件或写事件。

## 函数原型

- `int bufferevent_enable(struct bufferevent *bufev, short events);`

## 依赖头文件

- `#include <event2/bufferevent.h>`

## 输入参数

- `bufev`：目标 [[linux网络编程/概念词条/bufferevent|bufferevent]]。
- `events`：要启用的事件，通常来自 [[linux网络编程/概念词条/Libevent事件标志|Libevent事件标志]]，例如 `EV_READ`、`EV_WRITE`，也可以用按位或组合。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回 `0`。
- 失败返回 `-1`。

## 知识点补充

- 常见服务器代码会启用 `EV_READ`，表示有数据到来时触发读回调。
- 写事件通常不需要长期启用，因为 [[linux网络编程/函数笔记/Libevent/bufferevent_write|bufferevent_write]] 会把数据放入输出缓冲区，由 Libevent 负责发送。

## 常见用法

```c
bufferevent_enable(bev, EV_READ);
```

## 易错点

- 忘记启用 `EV_READ` 是读回调不触发的常见原因。
- `events` 不是 socket 的读写权限，而是 Libevent 层面的事件开关。

## 相关概念

- [[linux网络编程/概念词条/bufferevent|bufferevent]]
- [[linux网络编程/概念词条/Libevent事件标志|Libevent事件标志]]

## 相关课时

- [[linux网络编程/课时笔记/07 Libevent库/05 bufferevent基础|05 bufferevent基础]]

## 相关模块

- [[linux网络编程/07 Libevent库|07 Libevent库]]
