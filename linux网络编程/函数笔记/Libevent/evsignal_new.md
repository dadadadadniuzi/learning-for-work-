---
title: evsignal_new
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/Libevent
---
# evsignal_new

> [!info] 功能
> 创建一个专门监听信号的 Libevent 事件。它本质上是 `event_new` 的信号事件快捷写法。

## 函数原型

- `struct event *evsignal_new(struct event_base *base, int signum, event_callback_fn cb, void *arg);`

## 依赖头文件

- `#include <event2/event.h>`

## 输入参数

- `base`：事件所属的 [[linux网络编程/概念词条/event_base|event_base]]，由 [[linux网络编程/函数笔记/Libevent/event_base_new|event_base_new]] 创建。
- `signum`：要监听的信号编号，例如 `SIGINT`、`SIGTERM`。这些信号常见于 [[linux系统编程/07 信号|07 信号]] 相关知识。
- `cb`：信号事件触发时执行的回调函数，类型是 [[linux网络编程/概念词条/event_callback_fn|event_callback_fn]]。
- `arg`：传给回调函数的用户参数，可以传 `base`、结构体指针或 `NULL`。

## 输出参数

- 无直接输出参数。创建出的事件对象通过返回值交给调用者。

## 返回值

- 成功返回 `struct event *`。
- 失败返回 `NULL`。

## 它和 event_new 的关系

`evsignal_new()` 可以理解成下面这句的专门版本：

```c
event_new(base, signum, EV_SIGNAL | EV_PERSIST, cb, arg);
```

也就是说：

- 监听对象不再是普通 fd，而是信号编号。
- 事件标志核心是 [[linux网络编程/概念词条/Libevent事件标志|EV_SIGNAL]]。
- 通常会配合 `EV_PERSIST`，让信号事件触发后继续保留。

## 常见用法

```c
void sigint_cb(evutil_socket_t sig, short events, void *arg) {
    struct event_base *base = arg;
    event_base_loopbreak(base);
}

struct event *sigint_ev =
    evsignal_new(base, SIGINT, sigint_cb, base);

event_add(sigint_ev, NULL);
event_base_dispatch(base);
```

这个例子表示：

- 监听 `SIGINT`，也就是常见的 `Ctrl + C`。
- 收到信号后，执行回调。
- 回调里调用 `event_base_loopbreak` 结束事件循环。

## 回调参数怎么理解

信号事件触发时，回调函数原型仍然是：

```c
typedef void (*event_callback_fn)(evutil_socket_t fd, short what, void *arg);
```

但这里的第一个参数不再表示普通文件描述符，而是“触发的信号编号”。例如监听 `SIGINT` 时，这里通常会收到 `SIGINT` 对应的编号。

## 常见场景

- 捕获 `SIGINT`，优雅退出事件循环。
- 捕获 `SIGTERM`，在服务停止前清理资源。
- 在事件驱动程序中统一处理退出信号，而不是在阻塞式 `signal()` 逻辑里单独写。

## 易错点

- `evsignal_new()` 创建的是事件对象，仍然需要调用 [[linux网络编程/函数笔记/Libevent/event_add|event_add]] 才会真正加入监听。
- 回调里不要执行过重的耗时逻辑，最好只做“设置标志、退出循环、唤醒清理流程”这类简短工作。
- 它监听的是信号，不是 socket 读写事件，所以不要把 `signum` 当成普通 fd 使用。
- 如果事件循环没启动，信号事件也不会按 Libevent 的方式被分发。

## 相关函数

- [[linux网络编程/函数笔记/Libevent/event_new|event_new]]
- [[linux网络编程/函数笔记/Libevent/event_add|event_add]]
- [[linux网络编程/函数笔记/Libevent/event_base_dispatch|event_base_dispatch]]
- [[linux网络编程/函数笔记/Libevent/event_base_free|event_base_free]]

## 相关概念

- [[linux网络编程/概念词条/event_base|event_base]]
- [[linux网络编程/概念词条/event_callback_fn|event_callback_fn]]
- [[linux网络编程/概念词条/Libevent事件标志|Libevent事件标志]]
- [[linux网络编程/概念词条/Reactor反应堆模式|Reactor（反应堆模式）]]
- [[linux系统编程/07 信号|07 信号]]

## 相关课时

- [[linux网络编程/课时笔记/07 Libevent库/02 常规event基础|02 常规event基础]]

## 相关模块

- [[linux网络编程/07 Libevent库|07 Libevent库]]
