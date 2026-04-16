---
title: event_base_free
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/Libevent
---
# event_base_free

> [!info] 功能
> 释放由 [[linux网络编程/函数笔记/Libevent/event_base_new|event_base_new]] 创建的 [[linux网络编程/概念词条/event_base|event_base]]。

## 函数原型

- `void event_base_free(struct event_base *base);`

## 依赖头文件

- `#include <event2/event.h>`

## 输入参数

- `base`：要释放的事件循环对象。它应当来自 [[linux网络编程/函数笔记/Libevent/event_base_new|event_base_new]]，并且不应在释放后继续使用。

## 输出参数

- 无。

## 返回值

- 无返回值。

## 知识点补充

- 释放 `event_base` 表示事件循环对象生命周期结束。
- 课程代码中通常在 [[linux网络编程/函数笔记/Libevent/event_base_dispatch|event_base_dispatch]] 返回后释放它。
- 若还有事件、[[linux网络编程/概念词条/bufferevent|bufferevent]] 或 [[linux网络编程/概念词条/evconnlistener|evconnlistener]] 挂在上面，建议先释放这些对象，再释放 `event_base`。

## 常见用法

```c
event_base_free(base);
```

## 易错点

- 不要对同一个 `base` 重复释放。
- 释放后不要再把它传给其他 Libevent 函数。

## 相关概念

- [[linux网络编程/概念词条/event_base|event_base]]
- [[linux网络编程/概念词条/Libevent|Libevent]]

## 相关课时

- [[linux网络编程/课时笔记/07 Libevent库/02 常规event基础|02 常规event基础]]

## 相关模块

- [[linux网络编程/07 Libevent库|07 Libevent库]]
