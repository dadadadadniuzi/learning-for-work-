---
title: struct timeval
tags:
  - linux
  - 网络编程
  - 概念词条
  - 网络编程/IO多路复用
  - 网络编程/Libevent
---
# struct timeval

## 它是什么

[[linux网络编程/概念词条/struct timeval|struct timeval]] 是 Linux/Unix 中常见的“秒 + 微秒”时间结构，用来描述一段时间长度或一个时间值。

在网络编程课程里，它常见于两个位置：

- [[linux网络编程/函数笔记/IO多路复用/select|select]] 的 `timeout` 参数，用来控制最多阻塞多久。
- [[linux网络编程/函数笔记/Libevent/event_add|event_add]] 的 `timeout` 参数，用来设置事件超时时间。

## 依赖头文件

```c
#include <sys/time.h>
```

很多使用 [[linux网络编程/函数笔记/IO多路复用/select|select]] 的代码也会同时包含：

```c
#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>
```

## 结构形式

```c
struct timeval {
    long tv_sec;
    long tv_usec;
};
```

实际系统头文件中的字段类型可能会使用 `time_t`、`suseconds_t` 等类型，但学习时可以先抓住核心：一个字段表示秒，一个字段表示微秒。

## 字段含义

- `tv_sec`：秒。
- `tv_usec`：微秒，1 秒等于 1000000 微秒。

常见换算：

| 时间 | 写法 |
|---|---|
| 1 秒 | `tv_sec = 1; tv_usec = 0;` |
| 500 毫秒 | `tv_sec = 0; tv_usec = 500000;` |
| 1.5 秒 | `tv_sec = 1; tv_usec = 500000;` |

## 在 select 中的作用

[[linux网络编程/函数笔记/IO多路复用/select|select]] 的最后一个参数是：

```c
struct timeval *timeout
```

它决定 [[linux网络编程/函数笔记/IO多路复用/select|select]] 等待 fd 就绪时的阻塞方式：

| `timeout` 取值 | 含义 |
|---|---|
| `NULL` | 一直阻塞，直到有 fd 就绪或被信号打断 |
| `{0, 0}` | 不阻塞，立即检查一次并返回 |
| 正时间 | 最多等待指定时间，超时后返回 `0` |

示例：

```c
struct timeval tv;
tv.tv_sec = 5;
tv.tv_usec = 0;

int nready = select(maxfd + 1, &rset, NULL, NULL, &tv);
```

这表示最多阻塞 5 秒。如果 5 秒内没有 fd 就绪，[[linux网络编程/函数笔记/IO多路复用/select|select]] 返回 `0`。

## 在 Libevent 中的作用

[[linux网络编程/函数笔记/Libevent/event_add|event_add]] 的第二个参数也是 [[linux网络编程/概念词条/struct timeval|struct timeval]] 指针：

```c
int event_add(struct event *ev, const struct timeval *timeout);
```

- 传 `NULL`：不设置超时，只等待 fd 事件或信号事件。
- 传入具体时间：事件除了可以因为 fd 就绪触发，也可以因为超时触发。

示例：

```c
struct timeval tv;
tv.tv_sec = 5;
tv.tv_usec = 0;

event_add(ev, &tv);
```

这表示把事件加入事件循环，并设置 5 秒超时。

## 常见用法

立即轮询一次：

```c
struct timeval tv = {0, 0};
select(maxfd + 1, &rset, NULL, NULL, &tv);
```

等待 2.5 秒：

```c
struct timeval tv;
tv.tv_sec = 2;
tv.tv_usec = 500000;
select(maxfd + 1, &rset, NULL, NULL, &tv);
```

Libevent 设置 1 秒超时：

```c
struct timeval tv = {1, 0};
event_add(ev, &tv);
```

## 易错点

- `tv_usec` 是微秒，不是毫秒。500 毫秒要写 `500000`，不是 `500`。
- `tv_usec` 通常应保持在 `[0, 1000000)` 范围内，超过 1 秒的部分应进位到 `tv_sec`。
- [[linux网络编程/函数笔记/IO多路复用/select|select]] 在某些系统上可能会修改 `timeout`，循环中不要直接复用旧的 `struct timeval`，最好每轮重新赋值。
- `NULL` 和 `{0,0}` 不一样：`NULL` 是一直阻塞，`{0,0}` 是立即返回。

## 相关函数

- [[linux网络编程/函数笔记/IO多路复用/select|select]]
- [[linux网络编程/函数笔记/Libevent/event_add|event_add]]

## 相关概念

- [[linux网络编程/概念词条/select模型|select模型]]
- [[linux网络编程/概念词条/IO多路复用|IO多路复用]]
- [[linux网络编程/概念词条/event|event]]
- [[linux网络编程/概念词条/事件的未决与非未决|事件的未决与非未决]]

## 相关课时

- [[linux网络编程/课时笔记/05 IO多路复用/01 select|01 select]]
- [[linux网络编程/课时笔记/07 Libevent库/02 常规event基础|02 常规event基础]]
- [[linux网络编程/课时笔记/07 Libevent库/04 事件的未决与非未决|04 事件的未决与非未决]]

## 相关模块

- [[linux网络编程/05 IO多路复用|05 IO多路复用]]
- [[linux网络编程/07 Libevent库|07 Libevent库]]
