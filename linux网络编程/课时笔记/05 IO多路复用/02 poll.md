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
- [[linux网络编程/概念词条/pollfd|struct pollfd]]
- [[linux网络编程/概念词条/select poll epoll对比|select、poll、epoll 对比]]
- [[linux网络编程/概念词条/nfds_t|nfds_t]]
- `events` 和 `revents`
- [[linux网络编程/概念词条/poll事件宏|poll事件宏]]
- [[linux网络编程/函数笔记/IO多路复用/poll|poll]] 相比 [[linux网络编程/函数笔记/IO多路复用/select|select]] 的变化

## 本节学什么详解

- [[linux网络编程/概念词条/poll模型|poll模型]]：使用 [[linux网络编程/概念词条/pollfd|struct pollfd]] 数组描述要监听的 fd 和事件，由 [[linux网络编程/函数笔记/IO多路复用/poll|poll]] 返回哪些 fd 就绪。
- [[linux网络编程/概念词条/pollfd|struct pollfd]]：每个数组元素描述一个 fd，`events` 表示关心的事件，`revents` 表示内核返回的实际就绪事件。
- [[linux网络编程/概念词条/select poll epoll对比|select、poll、epoll 对比]]：帮助记住 poll 相比 select 改进了数据结构，但仍然需要每次传入数组并线性遍历。
- [[linux网络编程/概念词条/nfds_t|nfds_t]]：[[linux网络编程/函数笔记/IO多路复用/poll|poll]] 第二个参数的类型，表示要检查的数组元素数量，不是最大 fd 加 1。
- `events` 和 `revents`：应用程序写 `events`，内核写 `revents`，两者分离，比 [[linux网络编程/函数笔记/IO多路复用/select|select]] 修改原集合更直观。
- [[linux网络编程/概念词条/poll事件宏|poll事件宏]]：例如 `POLLIN` 表示可读，`POLLOUT` 表示可写，`POLLERR` 表示错误。
- 相比 [[linux网络编程/函数笔记/IO多路复用/select|select]]：[[linux网络编程/函数笔记/IO多路复用/poll|poll]] 不需要最大 fd + 1，也不受 [[linux网络编程/概念词条/fd_set|fd_set]] 位图大小限制，但仍然需要线性遍历数组。

## 知识点补充

- 常见读事件是 [[linux网络编程/概念词条/poll事件宏|POLLIN]]。
- 不使用的 [[linux网络编程/概念词条/pollfd|struct pollfd]] 元素可以把 `fd` 设置为 `-1`。
- [[linux网络编程/函数笔记/IO多路复用/poll|poll]] 的超时参数单位是毫秒，`-1` 表示一直阻塞，`0` 表示立即返回。

## 本节内容速览

- 准备 [[linux网络编程/概念词条/pollfd|struct pollfd]] 数组。
- 把监听 fd 和通信 fd 放入数组。
- 调用 [[linux网络编程/函数笔记/IO多路复用/poll|poll]] 等待就绪。
- 检查每个元素的 `revents`。

## 复习时要回答

- [[linux网络编程/函数笔记/IO多路复用/poll|poll]] 相比 [[linux网络编程/函数笔记/IO多路复用/select|select]] 解决了什么问题？
- `events` 和 `revents` 分别是谁写的？
- [[linux网络编程/概念词条/nfds_t|nfds_t]] 和 [[linux网络编程/函数笔记/IO多路复用/select|select]] 的 `nfds` 含义有什么不同？
- 为什么 [[linux网络编程/函数笔记/IO多路复用/poll|poll]] 仍然需要遍历数组？

## 本节关键函数

- [[linux网络编程/函数笔记/IO多路复用/poll|poll]]

## 本节关键概念

- [[linux网络编程/概念词条/poll模型|poll模型]]
- [[linux网络编程/概念词条/pollfd|struct pollfd]]
- [[linux网络编程/概念词条/select poll epoll对比|select、poll、epoll 对比]]
- [[linux网络编程/概念词条/nfds_t|nfds_t]]
- [[linux网络编程/概念词条/poll事件宏|poll事件宏]]
- [[linux网络编程/概念词条/事件就绪|事件就绪]]

## 关联模块

- [[linux网络编程/05 IO多路复用|05 IO多路复用]]
