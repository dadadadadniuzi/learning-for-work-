---
title: bufferevent基础
tags:
  - linux
  - 网络编程
  - 课时笔记
  - 网络编程/Libevent
---
# bufferevent基础

## 本节学什么

- [[linux网络编程/概念词条/bufferevent|bufferevent]]
- [[linux网络编程/概念词条/bufferevent读写缓冲区|bufferevent读写缓冲区]]
- [[linux网络编程/概念词条/bufferevent回调|bufferevent回调]]
- `bufferevent_socket_new`
- `bufferevent_setcb`
- `bufferevent_enable` / `bufferevent_disable`
- `bufferevent_read` / `bufferevent_write`

## 本节学什么详解

- [[linux网络编程/概念词条/bufferevent|bufferevent]]：Libevent 对 socket 读写事件和缓冲区的封装，比普通 `event` 更适合网络通信。
- [[linux网络编程/概念词条/bufferevent读写缓冲区|读写缓冲区]]：bufferevent 内部维护读缓冲区和写缓冲区，读回调中取数据，写函数把数据放入写缓冲区。
- [[linux网络编程/概念词条/bufferevent回调|bufferevent回调]]：包括读回调、写回调和事件回调，分别处理收到数据、写缓冲状态和连接事件。
- `bufferevent_socket_new`：创建基于 socket 的 bufferevent。
- `bufferevent_setcb`：设置读、写、事件回调。
- `bufferevent_enable/disable`：启用或禁用读写事件。
- `bufferevent_read/write`：从读缓冲区取数据，或向写缓冲区写入数据。

## 知识点补充

- `bufferevent_socket_new` 的 fd 应该是通信 fd，例如 `accept` 返回的 `cfd`，而不是监听 fd。
- `BEV_OPT_CLOSE_ON_FREE` 表示释放 bufferevent 时自动关闭关联 fd。
- 如果没有启用 `EV_READ`，即使设置了读回调，也不会按预期读取数据。

## 本节内容速览

- 创建 `bufferevent`。
- 设置回调函数。
- 启用读写事件。
- 在读回调里 `bufferevent_read`。
- 在需要发送时 `bufferevent_write`。
- 释放资源。

## 复习时要回答

- `bufferevent` 和普通 `event` 的侧重点有什么区别？
- 读回调、写回调、事件回调分别什么时候用？
- 为什么创建 bufferevent 时不能随便传监听 fd？

## 本节关键函数

- [[linux网络编程/函数笔记/Libevent/bufferevent_socket_new|bufferevent_socket_new]]
- [[linux网络编程/函数笔记/Libevent/bufferevent_setcb|bufferevent_setcb]]
- [[linux网络编程/函数笔记/Libevent/bufferevent_enable|bufferevent_enable]]
- [[linux网络编程/函数笔记/Libevent/bufferevent_disable|bufferevent_disable]]
- [[linux网络编程/函数笔记/Libevent/bufferevent_read|bufferevent_read]]
- [[linux网络编程/函数笔记/Libevent/bufferevent_write|bufferevent_write]]

## 本节关键概念

- [[linux网络编程/概念词条/bufferevent|bufferevent]]
- [[linux网络编程/概念词条/bufferevent读写缓冲区|bufferevent读写缓冲区]]
- [[linux网络编程/概念词条/bufferevent回调|bufferevent回调]]
- [[linux网络编程/概念词条/BEV_OPT_CLOSE_ON_FREE|BEV_OPT_CLOSE_ON_FREE]]

## 关联模块

- [[linux网络编程/07 Libevent库|07 Libevent库]]
