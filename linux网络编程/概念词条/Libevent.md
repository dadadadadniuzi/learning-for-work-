---
title: Libevent
tags:
  - linux
  - 网络编程
  - 概念词条
---
# Libevent

## 它是什么

- Libevent 是一个跨平台、事件驱动的网络库。
- 它封装了底层 IO 多路复用机制，让程序通过事件和回调组织网络通信。

## 怎么理解

- 手写 epoll 时，需要自己维护 fd、事件数组和循环。
- Libevent 把这些通用逻辑封装成 `event_base`、`event`、`bufferevent`、`evconnlistener` 等对象。

## 常见组成

- [[linux网络编程/概念词条/event_base|event_base]]
- [[linux网络编程/概念词条/event|event]]
- [[linux网络编程/概念词条/bufferevent|bufferevent]]
- [[linux网络编程/概念词条/evconnlistener|evconnlistener]]

## 常见出现位置

- [[linux网络编程/07 Libevent库|07 Libevent库]]
