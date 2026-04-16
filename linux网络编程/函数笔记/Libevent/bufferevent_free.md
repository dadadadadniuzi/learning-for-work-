---
title: bufferevent_free
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/Libevent
---
# bufferevent_free

> [!info] 功能
> 释放 [[linux网络编程/概念词条/bufferevent|bufferevent]] 对象，并根据创建时的选项决定是否关闭底层 socket。

## 函数原型

- `void bufferevent_free(struct bufferevent *bufev);`

## 依赖头文件

- `#include <event2/bufferevent.h>`

## 输入参数

- `bufev`：要释放的 bufferevent 对象。

## 输出参数

- 无。

## 返回值

- 无返回值。

## 知识点补充

- 如果创建时使用了 [[linux网络编程/概念词条/BEV_OPT_CLOSE_ON_FREE|BEV_OPT_CLOSE_ON_FREE]]，释放 bufferevent 时会同时关闭底层 socket。
- 常见做法是在事件回调发现对端关闭或发生错误时调用它。

## 常见用法

```c
bufferevent_free(bev);
```

## 易错点

- 不要释放后继续在读回调、写回调或事件回调中使用 `bev`。
- 如果没有设置自动关闭选项，底层 socket 的关闭要由程序自己负责。

## 相关概念

- [[linux网络编程/概念词条/bufferevent|bufferevent]]
- [[linux网络编程/概念词条/BEV_OPT_CLOSE_ON_FREE|BEV_OPT_CLOSE_ON_FREE]]

## 相关课时

- [[linux网络编程/课时笔记/07 Libevent库/05 bufferevent基础|05 bufferevent基础]]

## 相关模块

- [[linux网络编程/07 Libevent库|07 Libevent库]]
