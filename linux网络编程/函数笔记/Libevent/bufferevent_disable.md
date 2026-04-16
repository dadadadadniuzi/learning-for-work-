---
title: bufferevent_disable
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/Libevent
---
# bufferevent_disable

> [!info] 功能
> 关闭 [[linux网络编程/概念词条/bufferevent|bufferevent]] 的读事件或写事件监听。

## 函数原型

- `int bufferevent_disable(struct bufferevent *bufev, short events);`

## 依赖头文件

- `#include <event2/bufferevent.h>`

## 输入参数

- `bufev`：目标 [[linux网络编程/概念词条/bufferevent|bufferevent]]。
- `events`：要关闭的事件，例如 `EV_READ`、`EV_WRITE`，来自 [[linux网络编程/概念词条/Libevent事件标志|Libevent事件标志]]。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回 `0`。
- 失败返回 `-1`。

## 知识点补充

- 关闭 `EV_READ` 后，即使 socket 上有数据到达，读回调也不会再被调用。
- 这个函数只改变 Libevent 的事件监听状态，不关闭 socket，也不释放 bufferevent。

## 常见用法

```c
bufferevent_disable(bev, EV_READ);
```

## 易错点

- 不要把 disable 当作释放资源；释放资源应使用 [[linux网络编程/函数笔记/Libevent/bufferevent_free|bufferevent_free]]。
- 如果后面还想继续读，需要再次调用 [[linux网络编程/函数笔记/Libevent/bufferevent_enable|bufferevent_enable]]。

## 相关概念

- [[linux网络编程/概念词条/bufferevent|bufferevent]]
- [[linux网络编程/概念词条/Libevent事件标志|Libevent事件标志]]

## 相关课时

- [[linux网络编程/课时笔记/07 Libevent库/05 bufferevent基础|05 bufferevent基础]]

## 相关模块

- [[linux网络编程/07 Libevent库|07 Libevent库]]
