---
title: 常规event基础
tags:
  - linux
  - 网络编程
  - 课时笔记
  - 网络编程/Libevent
---
# 常规event基础

## 本节学什么

- [[linux网络编程/概念词条/event_base|event_base]]
- [[linux网络编程/概念词条/event|event]]
- `event_base_new`
- `event_new`
- `event_add`
- `event_base_dispatch`
- `event_free`

## 本节学什么详解

- [[linux网络编程/概念词条/event_base|event_base]]：Libevent 的事件底座，内部维护事件集合和底层多路复用机制。
- [[linux网络编程/概念词条/event|event]]：普通事件对象，用来描述“监听哪个 fd、监听什么事件、触发后调用哪个回调函数”。
- `event_base_new`：创建事件底座。
- `event_new`：创建事件对象并绑定 fd、事件标志、回调函数和回调参数。
- `event_add`：把事件加入底座，进入未决状态，等待触发。
- `event_base_dispatch`：启动事件循环。
- `event_free`：释放事件对象。

## 知识点补充

- 一个 `event_base` 可以管理多个 `event`。
- 常见事件标志有 `EV_READ`、`EV_WRITE`、`EV_PERSIST`。
- 不加 `EV_PERSIST` 的事件触发一次后可能需要重新添加。

## 本节内容速览

- 创建底座：`event_base_new`。
- 创建事件：`event_new`。
- 添加事件：`event_add`。
- 启动循环：`event_base_dispatch`。
- 释放资源：`event_free` 和 `event_base_free`。

## 复习时要回答

- Libevent 中一个普通事件从创建到进入循环的过程是什么？
- `event_base` 和 `event` 分别负责什么？
- `EV_PERSIST` 有什么作用？

## 本节关键函数

- [[linux网络编程/函数笔记/Libevent/event_base_new|event_base_new]]
- [[linux网络编程/函数笔记/Libevent/event_new|event_new]]
- [[linux网络编程/函数笔记/Libevent/event_add|event_add]]
- [[linux网络编程/函数笔记/Libevent/event_base_dispatch|event_base_dispatch]]
- [[linux网络编程/函数笔记/Libevent/event_free|event_free]]

## 本节关键概念

- [[linux网络编程/概念词条/event_base|event_base]]
- [[linux网络编程/概念词条/event|event]]
- [[linux网络编程/概念词条/Libevent事件标志|Libevent事件标志]]
- [[linux网络编程/概念词条/事件回调函数|事件回调函数]]

## 关联模块

- [[linux网络编程/07 Libevent库|07 Libevent库]]
