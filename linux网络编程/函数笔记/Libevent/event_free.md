---
title: event_free
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/Libevent
---
# event_free

> [!info] 功能
> 释放由 [[linux网络编程/函数笔记/Libevent/event_new|event_new]] 创建的普通 [[linux网络编程/概念词条/event|event]]。

## 函数原型

- `void event_free(struct event *ev);`

## 依赖头文件

- `#include <event2/event.h>`

## 输入参数

- `ev`：要释放的事件对象。它应当来自 [[linux网络编程/函数笔记/Libevent/event_new|event_new]]。

## 输出参数

- 无。

## 返回值

- 无返回值。

## 知识点补充

- 如果事件仍处于 [[linux网络编程/概念词条/事件的未决与非未决|未决]] 状态，释放前 Libevent 会处理事件对象的清理。
- `event_free` 释放的是 Libevent 的事件对象，不一定关闭对应的 `fd`；文件描述符是否关闭要看程序自己管理。

## 常见用法

```c
event_free(ev);
```

## 易错点

- 不要释放后继续使用 `ev`。
- 不要把 `event_free` 当作 `close` 使用；普通 `event` 不自动关闭文件描述符。

## 相关概念

- [[linux网络编程/概念词条/event|event]]
- [[linux网络编程/概念词条/事件的未决与非未决|事件的未决与非未决]]

## 相关课时

- [[linux网络编程/课时笔记/07 Libevent库/02 常规event基础|02 常规event基础]]

## 相关模块

- [[linux网络编程/07 Libevent库|07 Libevent库]]
