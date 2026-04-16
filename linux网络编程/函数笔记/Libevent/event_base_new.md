---
title: event_base_new
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/Libevent
---
# event_base_new

> [!info] 功能
> 创建一个 [[linux网络编程/概念词条/event_base|event_base]]，它是 Libevent 的“事件循环核心”，后续创建的 [[linux网络编程/概念词条/event|event]]、[[linux网络编程/概念词条/bufferevent|bufferevent]]、[[linux网络编程/概念词条/evconnlistener|evconnlistener]] 都要挂到它上面。

## 函数原型

- `struct event_base *event_base_new(void);`

## 依赖头文件

- `#include <event2/event.h>`

## 输入参数

- 无输入参数。调用时 Libevent 会根据当前系统能力选择合适的后端机制，例如 Linux 上通常可能使用 epoll。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回一个 `struct event_base *` 指针。
- 失败返回 `NULL`，常见原因是内存不足或 Libevent 后端初始化失败。

## 知识点补充

- [[linux网络编程/概念词条/event_base|event_base]] 保存事件集合、底层 IO 复用后端、定时器队列和循环状态。
- 一个程序可以有多个 `event_base`，但入门阶段通常一个线程使用一个 `event_base`。
- 创建成功后一般配合 [[linux网络编程/函数笔记/Libevent/event_base_dispatch|event_base_dispatch]] 进入循环，结束前用 [[linux网络编程/函数笔记/Libevent/event_base_free|event_base_free]] 释放。

## 常见用法

```c
struct event_base *base = event_base_new();
if (base == NULL) {
    return -1;
}
```

## 易错点

- 只创建 `event_base` 不会自动监听任何文件描述符，还必须创建事件并调用 [[linux网络编程/函数笔记/Libevent/event_add|event_add]]。
- 释放事件循环前，应先处理或释放挂在上面的事件对象，避免资源生命周期混乱。

## 相关概念

- [[linux网络编程/概念词条/Libevent|Libevent]]
- [[linux网络编程/概念词条/event_base|event_base]]
- [[linux网络编程/概念词条/IO多路复用|IO多路复用]]

## 相关课时

- [[linux网络编程/课时笔记/07 Libevent库/01 Libevent简介与安装|01 Libevent简介与安装]]
- [[linux网络编程/课时笔记/07 Libevent库/02 常规event基础|02 常规event基础]]

## 相关模块

- [[linux网络编程/07 Libevent库|07 Libevent库]]
