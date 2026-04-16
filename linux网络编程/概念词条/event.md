---
title: event
tags:
  - linux
  - 网络编程
  - 概念词条
---
# event

## 它是什么

- `event` 是 Libevent 中的普通事件对象。
- 它描述“监听哪个 fd、监听什么事件、触发后调用哪个回调函数、给回调传什么参数”。

## 怎么理解

- `event_base` 是事件循环底座。
- `event` 是挂到底座上的具体监听任务。
- `event_add` 后，事件进入未决状态，才会被事件循环关注。

## 常见相关函数

- [[linux网络编程/函数笔记/Libevent/event_new|event_new]]
- [[linux网络编程/函数笔记/Libevent/event_add|event_add]]
- [[linux网络编程/函数笔记/Libevent/event_free|event_free]]

## 易错点

- `event_new` 只创建事件，不会自动监听。
- 如果事件不是持久事件，触发一次后可能需要重新添加。

## 常见出现位置

- [[linux网络编程/课时笔记/07 Libevent库/02 常规event基础|02 常规event基础]]
- [[linux网络编程/课时笔记/07 Libevent库/03 event实现本地通信|03 event实现本地通信]]
