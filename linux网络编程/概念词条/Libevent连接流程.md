---
title: Libevent连接流程
tags:
  - linux
  - 网络编程
  - 概念词条
---
# Libevent连接流程

## 它是什么

- Libevent 连接流程指使用 `event_base`、`evconnlistener` 和 `bufferevent` 组织 TCP 服务端和客户端通信的过程。

## 服务端流程

- 创建 `event_base`。
- 使用 `evconnlistener_new_bind` 创建监听器。
- 新连接到来时触发 listener 回调。
- 在 listener 回调中用已连接 fd 创建 `bufferevent`。
- 设置 bufferevent 回调并启用读事件。
- 启动 `event_base_dispatch`。

## 客户端流程

- 创建 `event_base`。
- 创建 `bufferevent_socket_new`。
- 设置 bufferevent 回调。
- 调用 `bufferevent_socket_connect` 连接服务器。
- 启用读写事件并启动事件循环。

## 易错点

- 服务端 listener 回调里的 fd 是通信 fd，不是监听 fd。
- 客户端连接成功与否通常通过 bufferevent 的事件回调处理。

## 常见出现位置

- [[linux网络编程/课时笔记/07 Libevent库/06 evconnlistener与通信流程|06 evconnlistener与通信流程]]
