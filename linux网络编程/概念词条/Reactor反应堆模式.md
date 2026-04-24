---
title: Reactor反应堆模式
tags:
  - linux
  - 网络编程
  - 概念词条
  - 网络编程/IO多路复用
  - 网络编程/Libevent
---
# Reactor（反应堆模式）

## 它是什么

Reactor，也叫反应堆模式，是一种事件驱动编程模型。

它的核心思想是：程序不为每个连接都阻塞等待，而是把 fd 注册到一个事件监听机制中；当某个 fd 发生可读、可写、连接到来等事件时，由事件循环分发给对应的处理函数。

在 Linux 网络编程里，Reactor 通常建立在 [[linux网络编程/概念词条/IO多路复用|IO多路复用]] 之上，例如 [[linux网络编程/函数笔记/IO多路复用/select|select]]、[[linux网络编程/函数笔记/IO多路复用/poll|poll]]、[[linux网络编程/概念词条/epoll模型|epoll模型]]。

## 为什么需要 Reactor

阻塞式服务器里，一个线程如果卡在 [[linux网络编程/函数笔记/Socket/accept|accept]] 或 [[linux网络编程/函数笔记/Socket/read|read]] 上，就很难同时照顾其他连接。

Reactor 把程序改成这种思路：

```text
不主动阻塞等某一个 fd
  ↓
把多个 fd 注册到事件监听器
  ↓
事件循环等待“谁就绪”
  ↓
哪个 fd 就绪，就调用对应处理函数
```

这样一个线程就可以管理很多连接。

## 核心组成

Reactor 模式常见组成：

- 事件源：产生事件的对象，例如监听 socket、客户端 socket、定时器。
- 事件：可读、可写、新连接、超时、错误等。
- 事件分离器：负责等待事件并返回就绪事件，底层可以是 [[linux网络编程/函数笔记/IO多路复用/select|select]]、[[linux网络编程/函数笔记/IO多路复用/poll|poll]]、[[linux网络编程/函数笔记/IO多路复用/epoll_wait|epoll_wait]]。
- 事件处理器：事件发生后执行的处理逻辑，例如 accept 回调、读回调、写回调。
- 事件循环：循环等待事件、分发事件、执行处理器。

## 基本流程

```text
初始化 Reactor
  ↓
注册监听 fd 和对应处理函数
  ↓
进入事件循环
  ↓
IO 多路复用等待事件就绪
  ↓
分发事件
  ↓
调用对应回调处理事件
  ↓
继续事件循环
```

## 用 epoll 理解 Reactor

手写 epoll 服务器其实已经有 Reactor 的影子：

```text
epoll_create
  创建事件监听器

epoll_ctl
  注册 fd 和关心的事件

epoll_wait
  等待就绪事件

遍历 events
  根据 fd 和事件类型调用 accept/read/write 处理逻辑
```

其中：

- [[linux网络编程/函数笔记/IO多路复用/epoll_wait|epoll_wait]] 相当于事件分离器。
- `events[i]` 是就绪事件。
- `accept/read/write` 逻辑是事件处理器。
- `while` 循环就是事件循环。

## 和 Libevent 的关系

[[linux网络编程/概念词条/Libevent|Libevent]] 可以理解成对 Reactor 模式的工程化封装。

对应关系：

| Reactor 概念 | Libevent 中的对应物 |
|---|---|
| Reactor / 事件循环核心 | [[linux网络编程/概念词条/event_base|event_base]] |
| 事件源和事件描述 | [[linux网络编程/概念词条/event|event]]、[[linux网络编程/概念词条/bufferevent|bufferevent]] |
| 事件注册 | [[linux网络编程/函数笔记/Libevent/event_add|event_add]]、[[linux网络编程/函数笔记/Libevent/bufferevent_enable|bufferevent_enable]] |
| 事件分发循环 | [[linux网络编程/函数笔记/Libevent/event_base_dispatch|event_base_dispatch]] |
| 事件处理器 | [[linux网络编程/概念词条/事件回调函数|事件回调函数]]、[[linux网络编程/概念词条/bufferevent回调|bufferevent回调]] |

所以学习 Libevent 时，如果能先理解 Reactor，就会更容易理解为什么它总是围绕“事件、回调、事件循环”组织代码。

## 单 Reactor 模型

单 Reactor 指一个线程负责所有事情：

```text
一个事件循环
  负责 accept
  负责 read
  负责 write
  负责业务处理
```

优点：

- 模型简单。
- 没有复杂线程同步。
- 适合轻量连接或学习阶段。

缺点：

- 如果某个回调执行太久，会阻塞整个事件循环。
- CPU 密集任务或耗时业务会拖慢所有连接。

## Reactor + 线程池

更常见的高并发模型是 Reactor + [[linux网络编程/概念词条/线程池|线程池]]：

```text
Reactor 线程
  负责监听 fd 就绪
  负责 accept
  负责把耗时任务投递到线程池

线程池
  负责业务处理
  处理完后把结果交回 Reactor 或直接写回
```

这种方式把“事件检测”和“业务处理”分开，可以避免耗时任务卡住事件循环。

## 和 Proactor 的简单区别

初学阶段只需要粗略知道：

- Reactor：事件就绪后通知应用程序，应用程序自己执行读写。
- Proactor：异步操作完成后通知应用程序，读写结果已经完成。

Linux 常见的 [[linux网络编程/概念词条/epoll模型|epoll]] + 回调服务器更接近 Reactor 思路。

## 易错点

- Reactor 不是某个具体函数，而是一种组织网络程序的模式。
- [[linux网络编程/概念词条/epoll模型|epoll]] 是底层 IO 多路复用机制，Reactor 是基于它组织事件分发的设计模式。
- 回调函数不要长时间阻塞，否则单 Reactor 的整个事件循环都会被拖慢。
- Reactor 和线程池不是互斥关系，常常组合使用。
- Libevent 不是 Reactor 本身，但它实现了典型事件驱动/Reactor 风格的封装。

## 相关函数

- [[linux网络编程/函数笔记/IO多路复用/select|select]]
- [[linux网络编程/函数笔记/IO多路复用/poll|poll]]
- [[linux网络编程/函数笔记/IO多路复用/epoll_create|epoll_create]]
- [[linux网络编程/函数笔记/IO多路复用/epoll_ctl|epoll_ctl]]
- [[linux网络编程/函数笔记/IO多路复用/epoll_wait|epoll_wait]]
- [[linux网络编程/函数笔记/Libevent/event_base_dispatch|event_base_dispatch]]
- [[linux网络编程/函数笔记/Libevent/event_add|event_add]]

## 相关概念

- [[linux网络编程/概念词条/IO多路复用|IO多路复用]]
- [[linux网络编程/概念词条/epoll模型|epoll模型]]
- [[linux网络编程/概念词条/Libevent|Libevent]]
- [[linux网络编程/概念词条/event_base|event_base]]
- [[linux网络编程/概念词条/事件回调函数|事件回调函数]]
- [[linux网络编程/概念词条/bufferevent|bufferevent]]
- [[linux网络编程/概念词条/线程池|线程池]]

## 相关课时

- [[linux网络编程/课时笔记/05 IO多路复用/03 epoll|03 epoll]]
- [[linux网络编程/课时笔记/07 Libevent库/01 Libevent简介与安装|01 Libevent简介与安装]]
- [[linux网络编程/课时笔记/07 Libevent库/02 常规event基础|02 常规event基础]]
- [[linux网络编程/课时笔记/07 Libevent库/05 bufferevent基础|05 bufferevent基础]]

## 相关模块

- [[linux网络编程/05 IO多路复用|05 IO多路复用]]
- [[linux网络编程/07 Libevent库|07 Libevent库]]
