---
title: Libevent事件标志
tags:
  - linux
  - 网络编程
  - 概念词条
---
# Libevent事件标志

## 它是什么

- Libevent 事件标志用于说明事件对象要监听什么类型的事件。

## 常见标志

- `EV_READ`：监听可读事件。
- `EV_WRITE`：监听可写事件。
- `EV_PERSIST`：事件触发后不自动移除，继续保持监听。

## 怎么理解

- `EV_READ` 常用于 socket、管道、FIFO 有数据可读。
- `EV_WRITE` 常用于 fd 可写，但写事件很容易持续就绪，要谨慎使用。
- `EV_PERSIST` 让事件变成持久事件。

## 易错点

- 事件标志可以按位或组合，例如 `EV_READ | EV_PERSIST`。
- 如果忘记 `EV_PERSIST`，事件触发后可能不再继续监听。

## 常见出现位置

- [[linux网络编程/函数笔记/Libevent/event_new|event_new]]
- [[linux网络编程/函数笔记/Libevent/bufferevent_enable|bufferevent_enable]]
