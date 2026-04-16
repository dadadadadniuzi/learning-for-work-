---
title: struct timeval
tags:
  - linux
  - 网络编程
  - 概念词条
  - 网络编程/Libevent
---
# struct timeval

`struct timeval` 是 Linux/Unix 中常见的“秒 + 微秒”时间结构，Libevent 的 [[linux网络编程/函数笔记/Libevent/event_add|event_add]] 可以用它设置事件超时时间。

## 结构形式

```c
struct timeval {
    long tv_sec;
    long tv_usec;
};
```

## 字段含义

- `tv_sec`：秒。
- `tv_usec`：微秒，1 秒等于 1000000 微秒。

## 在 Libevent 中的作用

- [[linux网络编程/函数笔记/Libevent/event_add|event_add]] 的第二个参数传 `NULL`，表示不设置超时。
- 传入 `struct timeval` 指针，表示事件除了等待 fd 就绪外，也可以在超时后触发。

## 常见用法

```c
struct timeval tv;
tv.tv_sec = 5;
tv.tv_usec = 0;
event_add(ev, &tv);
```

## 易错点

- `tv_usec` 是微秒，不是毫秒。
- 如果想表示 500 毫秒，应写 `tv_sec = 0`、`tv_usec = 500000`。

## 相关函数

- [[linux网络编程/函数笔记/Libevent/event_add|event_add]]

## 相关模块

- [[linux网络编程/07 Libevent库|07 Libevent库]]
