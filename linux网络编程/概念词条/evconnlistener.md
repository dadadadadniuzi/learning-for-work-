---
title: evconnlistener
tags:
  - linux
  - 网络编程
  - 概念词条
---
# evconnlistener

## 它是什么

- `evconnlistener` 是 Libevent 提供的连接监听器对象。
- 它用于服务端监听客户端连接，并在新连接到来时调用回调函数。

## 怎么理解

- 传统 TCP 服务端要手写 `socket/bind/listen/accept`。
- `evconnlistener_new_bind` 把这几步封装起来。
- 每当有新客户端连接，listener 回调会收到一个已连接 fd。

## 常见相关函数

- [[linux网络编程/函数笔记/Libevent/evconnlistener_new_bind|evconnlistener_new_bind]]
- [[linux网络编程/函数笔记/Libevent/evconnlistener_free|evconnlistener_free]]

## 易错点

- listener 负责接收连接，不负责直接读写客户端数据。
- 客户端通信通常在 listener 回调里创建 `bufferevent` 来处理。

## 常见出现位置

- [[linux网络编程/课时笔记/07 Libevent库/06 evconnlistener与通信流程|06 evconnlistener与通信流程]]
