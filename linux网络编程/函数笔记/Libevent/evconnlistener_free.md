---
title: evconnlistener_free
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/Libevent
---
# evconnlistener_free

> [!info] 功能
> 释放 [[linux网络编程/概念词条/evconnlistener|evconnlistener]] 监听对象。

## 函数原型

- `void evconnlistener_free(struct evconnlistener *lev);`

## 依赖头文件

- `#include <event2/listener.h>`

## 输入参数

- `lev`：要释放的 listener 对象，通常来自 [[linux网络编程/函数笔记/Libevent/evconnlistener_new_bind|evconnlistener_new_bind]]。

## 输出参数

- 无。

## 返回值

- 无返回值。

## 知识点补充

- 如果创建 listener 时使用了 [[linux网络编程/概念词条/LEV_OPT_CLOSE_ON_FREE与LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE]]，释放 listener 时会关闭底层监听 socket。
- 释放 listener 只是不再接收新的连接，不会自动释放已经建立连接的 [[linux网络编程/概念词条/bufferevent|bufferevent]]。

## 常见用法

```c
evconnlistener_free(listener);
```

## 易错点

- 不要把 listener 和客户端通信的 bufferevent 混在一起释放。
- 释放 listener 后，已经 accept 出来的客户端连接仍需要各自管理。

## 相关概念

- [[linux网络编程/概念词条/evconnlistener|evconnlistener]]
- [[linux网络编程/概念词条/LEV_OPT_CLOSE_ON_FREE与LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE 与 LEV_OPT_REUSEABLE]]

## 相关课时

- [[linux网络编程/课时笔记/07 Libevent库/06 evconnlistener与通信流程|06 evconnlistener与通信流程]]

## 相关模块

- [[linux网络编程/07 Libevent库|07 Libevent库]]
