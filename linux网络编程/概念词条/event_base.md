---
title: event_base
tags:
  - linux
  - 网络编程
  - 概念词条
---
# event_base

## 它是什么

- `event_base` 是 Libevent 的事件底座，也可以理解成事件循环的核心对象。
- 它内部维护事件集合，并选择合适的底层 IO 多路复用机制，例如 epoll、poll、select。

## 怎么理解

- 普通 `event` 和 `bufferevent` 都需要挂到某个 `event_base` 上。
- 启动 `event_base_dispatch` 后，程序进入事件循环，等待事件触发并调用回调。

## 常见相关函数

- [[linux网络编程/函数笔记/Libevent/event_base_new|event_base_new]]
- [[linux网络编程/函数笔记/Libevent/event_base_dispatch|event_base_dispatch]]

## 易错点

- 创建事件对象不等于启动事件循环。
- 一个程序可以有多个 `event_base`，但初学阶段通常先掌握一个底座管理多个事件。

## 常见出现位置

- [[linux网络编程/07 Libevent库|07 Libevent库]]
- [[linux网络编程/课时笔记/07 Libevent库/02 常规event基础|02 常规event基础]]
