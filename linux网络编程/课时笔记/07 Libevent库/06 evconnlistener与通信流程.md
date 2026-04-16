---
title: evconnlistener与通信流程
tags:
  - linux
  - 网络编程
  - 课时笔记
  - 网络编程/Libevent
---
# evconnlistener与通信流程

## 本节学什么

- [[linux网络编程/概念词条/evconnlistener|evconnlistener]]
- `evconnlistener_new_bind`
- `evconnlistener_free`
- `bufferevent_socket_connect`
- Libevent 服务端和客户端通信流程

## 本节学什么详解

- [[linux网络编程/概念词条/evconnlistener|evconnlistener]]：Libevent 提供的连接监听器，封装服务端创建 socket、绑定、监听、接收连接等流程。
- `evconnlistener_new_bind`：创建监听器并绑定服务器地址，内部完成部分传统 socket 服务端初始化工作。
- `evconnlistener_free`：释放监听器资源。
- `bufferevent_socket_connect`：Libevent 客户端常用连接函数，让 bufferevent 关联的 socket 主动连接服务器。
- Libevent 通信流程：服务端创建 `event_base`，创建 listener，连接到来时在 listener 回调中创建 bufferevent，再设置回调和启用读写。

## 知识点补充

- listener 回调触发时，说明有新的客户端连接。
- listener 回调中的 fd 是已经连接好的通信 fd，常用它创建 `bufferevent_socket_new`。
- 客户端使用 `bufferevent_socket_connect` 后，连接成功、断开、错误等会通过事件回调通知。

## 本节内容速览

- 服务端：`event_base_new`。
- 服务端：`evconnlistener_new_bind`。
- listener 回调：创建 bufferevent。
- bufferevent：设置回调，启用读事件。
- 客户端：创建 bufferevent，调用 `bufferevent_socket_connect`。
- 启动事件循环。

## 复习时要回答

- `evconnlistener_new_bind` 替我们封装了传统 socket 服务端哪些步骤？
- listener 回调里的 fd 应该怎么处理？
- Libevent 客户端为什么要用 `bufferevent_socket_connect`？

## 本节关键函数

- [[linux网络编程/函数笔记/Libevent/evconnlistener_new_bind|evconnlistener_new_bind]]
- [[linux网络编程/函数笔记/Libevent/evconnlistener_free|evconnlistener_free]]
- [[linux网络编程/函数笔记/Libevent/bufferevent_socket_new|bufferevent_socket_new]]
- [[linux网络编程/函数笔记/Libevent/bufferevent_socket_connect|bufferevent_socket_connect]]
- [[linux网络编程/函数笔记/Libevent/bufferevent_setcb|bufferevent_setcb]]

## 本节关键概念

- [[linux网络编程/概念词条/evconnlistener|evconnlistener]]
- [[linux网络编程/概念词条/bufferevent|bufferevent]]
- [[linux网络编程/概念词条/bufferevent回调|bufferevent回调]]
- [[linux网络编程/概念词条/Libevent连接流程|Libevent连接流程]]

## 关联模块

- [[linux网络编程/07 Libevent库|07 Libevent库]]
