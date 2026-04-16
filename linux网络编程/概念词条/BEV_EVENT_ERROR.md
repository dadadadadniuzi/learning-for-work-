---
title: BEV_EVENT_ERROR
tags:
  - linux
  - 网络编程
  - 概念词条
  - 网络编程/Libevent
---
# BEV_EVENT_ERROR

`BEV_EVENT_ERROR` 表示 bufferevent 遇到错误，例如连接失败、网络异常或底层 socket 错误。

## 常见处理

```c
if (events & BEV_EVENT_ERROR) {
    bufferevent_free(bev);
}
```

## 学习重点

- 它通常在 [[linux网络编程/概念词条/bufferevent_event_cb|bufferevent_event_cb]] 中判断。
- 发生错误后通常需要释放当前连接对应的 [[linux网络编程/概念词条/bufferevent|bufferevent]]。

## 相关函数

- [[linux网络编程/函数笔记/Libevent/bufferevent_setcb|bufferevent_setcb]]
- [[linux网络编程/函数笔记/Libevent/bufferevent_free|bufferevent_free]]

## 相关模块

- [[linux网络编程/07 Libevent库|07 Libevent库]]
