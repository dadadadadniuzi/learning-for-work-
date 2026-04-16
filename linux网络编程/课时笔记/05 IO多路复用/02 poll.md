---
title: poll
tags:
  - linux
  - 网络编程
  - 课时笔记
  - 网络编程/IO多路复用
---
# poll

## 本节学什么

- [[linux网络编程/概念词条/poll模型|poll模型]]
- [[linux网络编程/概念词条/pollfd|pollfd]]
- `events` 和 `revents`
- `poll` 相比 `select` 的变化

## 本节学什么详解

- [[linux网络编程/概念词条/poll模型|poll模型]]：使用 `pollfd` 数组描述要监听的 fd 和事件，由 `poll` 返回哪些 fd 就绪。
- [[linux网络编程/概念词条/pollfd|pollfd]]：每个数组元素描述一个 fd，`events` 表示关心的事件，`revents` 表示内核返回的实际就绪事件。
- `events` 和 `revents`：应用程序写 `events`，内核写 `revents`，两者分离，比 `select` 修改原集合更直观。
- 相比 `select`：`poll` 不需要最大 fd + 1，也不受 `fd_set` 位图大小限制，但仍然需要线性遍历数组。

## 知识点补充

- 常见读事件是 `POLLIN`。
- 不使用的 `pollfd` 元素可以把 `fd` 设置为 `-1`。
- `poll` 的超时参数单位是毫秒，`-1` 表示一直阻塞，`0` 表示立即返回。

## 本节内容速览

- 准备 `pollfd` 数组。
- 把监听 fd 和通信 fd 放入数组。
- 调用 `poll` 等待就绪。
- 检查每个元素的 `revents`。

## 复习时要回答

- `poll` 相比 `select` 解决了什么问题？
- `events` 和 `revents` 分别是谁写的？
- 为什么 `poll` 仍然需要遍历数组？

## 本节关键函数

- [[linux网络编程/函数笔记/IO多路复用/poll|poll]]

## 本节关键概念

- [[linux网络编程/概念词条/poll模型|poll模型]]
- [[linux网络编程/概念词条/pollfd|pollfd]]
- [[linux网络编程/概念词条/事件就绪|事件就绪]]

## 关联模块

- [[linux网络编程/05 IO多路复用|05 IO多路复用]]
