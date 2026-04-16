---
title: event实现本地通信
tags:
  - linux
  - 网络编程
  - 课时笔记
  - 网络编程/Libevent
---
# event实现本地通信

## 本节学什么

- event 和[[linux网络编程/概念词条/本地通信|本地通信]]结合
- 使用 event 监听 FIFO 或本地 fd
- `EV_READ` / `EV_WRITE` / `EV_PERSIST`
- 回调函数读取或写入数据

## 本节学什么详解

- event 和本地通信结合：Libevent 不只能处理网络 socket，也能监听管道、FIFO 等本地 fd 的事件。
- 监听 FIFO 或本地 fd：先准备 fd，再用 `event_new` 绑定 fd 和事件，再用 `event_add` 加入事件循环。
- `EV_READ/EV_WRITE/EV_PERSIST`：分别表示可读事件、可写事件和持续触发事件。
- 回调函数：当 fd 满足监听条件时，Libevent 调用你注册的回调函数，在回调里执行 `read/write` 等操作。

## 知识点补充

- 如果读端使用 `EV_PERSIST`，事件触发后仍然保留在事件循环中。
- 写事件通常很容易一直就绪，使用 `EV_WRITE | EV_PERSIST` 时要小心持续触发。
- 本节适合把系统编程里的 FIFO 和网络编程里的事件循环连接起来理解。

## 本节内容速览

- 创建本地通信 fd。
- 创建 `event_base`。
- 创建 `event` 并绑定 fd、事件和回调。
- `event_add` 添加事件。
- `event_base_dispatch` 启动循环。

## 复习时要回答

- 为什么 Libevent 可以处理本地通信 fd？
- `EV_READ` 和 `EV_WRITE` 分别表示什么？
- 持续触发事件为什么要小心？

## 本节关键函数

- [[linux网络编程/函数笔记/Libevent/event_new|event_new]]
- [[linux网络编程/函数笔记/Libevent/event_add|event_add]]
- [[linux网络编程/函数笔记/Libevent/event_base_dispatch|event_base_dispatch]]

## 本节关键概念

- [[linux网络编程/概念词条/event|event]]
- [[linux网络编程/概念词条/Libevent事件标志|Libevent事件标志]]
- [[linux网络编程/概念词条/事件回调函数|事件回调函数]]
- [[linux网络编程/概念词条/本地通信|本地通信]]

## 关联模块

- [[linux网络编程/07 Libevent库|07 Libevent库]]
