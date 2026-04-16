---
title: bufferevent_setcb
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/Libevent
---
# bufferevent_setcb

> [!info] 功能
> 给 [[linux网络编程/概念词条/bufferevent|bufferevent]] 设置读回调、写回调和事件回调。

## 函数原型

- `void bufferevent_setcb(struct bufferevent *bufev, bufferevent_data_cb readcb, bufferevent_data_cb writecb, bufferevent_event_cb eventcb, void *cbarg);`

## 依赖头文件

- `#include <event2/bufferevent.h>`

## 输入参数

- `bufev`：目标 [[linux网络编程/概念词条/bufferevent|bufferevent]]。
- `readcb`：读回调，类型是 [[linux网络编程/概念词条/bufferevent_data_cb|bufferevent_data_cb]]。当读缓冲区中有数据可读时触发。
- `writecb`：写回调，类型同样是 [[linux网络编程/概念词条/bufferevent_data_cb|bufferevent_data_cb]]。当写缓冲区数据被发送到足够低的水位时触发；入门阶段常传 `NULL`。
- `eventcb`：事件回调，类型是 [[linux网络编程/概念词条/bufferevent_event_cb|bufferevent_event_cb]]。连接成功、连接失败、对端关闭、错误等事件会触发它。
- `cbarg`：传给所有回调函数的自定义参数。

## 输出参数

- 无直接输出参数。

## 返回值

- 无返回值。

## 知识点补充

- 读回调负责从输入缓冲区中取数据，常用 [[linux网络编程/函数笔记/Libevent/bufferevent_read|bufferevent_read]]。
- 写数据通常直接调用 [[linux网络编程/函数笔记/Libevent/bufferevent_write|bufferevent_write]]，写回调不一定每个程序都需要。
- 事件回调要重点处理 [[linux网络编程/概念词条/BEV_EVENT_CONNECTED|BEV_EVENT_CONNECTED]]、[[linux网络编程/概念词条/BEV_EVENT_EOF|BEV_EVENT_EOF]]、[[linux网络编程/概念词条/BEV_EVENT_ERROR|BEV_EVENT_ERROR]]。

## 常见用法

```c
bufferevent_setcb(bev, read_cb, NULL, event_cb, NULL);
```

## 易错点

- 设置回调后还要调用 [[linux网络编程/函数笔记/Libevent/bufferevent_enable|bufferevent_enable]]，否则读事件可能不会触发。
- 回调函数的参数列表必须和对应 typedef 完全匹配。

## 相关概念

- [[linux网络编程/概念词条/bufferevent回调|bufferevent回调]]
- [[linux网络编程/概念词条/bufferevent_data_cb|bufferevent_data_cb]]
- [[linux网络编程/概念词条/bufferevent_event_cb|bufferevent_event_cb]]
- [[linux网络编程/概念词条/bufferevent读写缓冲区|bufferevent读写缓冲区]]

## 相关课时

- [[linux网络编程/课时笔记/07 Libevent库/05 bufferevent基础|05 bufferevent基础]]

## 相关模块

- [[linux网络编程/07 Libevent库|07 Libevent库]]
