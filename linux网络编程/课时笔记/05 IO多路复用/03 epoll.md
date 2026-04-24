---
title: epoll
tags:
  - linux
  - 网络编程
  - 课时笔记
  - 网络编程/IO多路复用
---
# epoll

## 本节学什么

- [[linux网络编程/概念词条/epoll模型|epoll模型]]
- [[linux网络编程/概念词条/select poll epoll对比|select、poll、epoll 对比]]
- [[linux网络编程/概念词条/Reactor反应堆模式|Reactor（反应堆模式）]]
- `epoll_create`
- `epoll_ctl`
- `epoll_wait`
- [[linux网络编程/概念词条/epoll_event|epoll_event]]
- [[linux网络编程/概念词条/epoll_data_t|epoll_data_t]]
- [[linux网络编程/概念词条/水平触发与边沿触发|水平触发与边沿触发]]

## 本节学什么详解

- [[linux网络编程/概念词条/epoll模型|epoll模型]]：把需要关注的 fd 注册到内核 epoll 实例中，之后通过 [[linux网络编程/函数笔记/IO多路复用/epoll_wait|epoll_wait]] 获取就绪事件。
- [[linux网络编程/概念词条/select poll epoll对比|select、poll、epoll 对比]]：帮助记住 epoll 把注册和等待拆开，只返回就绪事件，更适合大量连接。
- [[linux网络编程/概念词条/Reactor反应堆模式|Reactor（反应堆模式）]]：把 epoll 等 IO 多路复用机制组织成事件循环，事件就绪后分发给对应处理函数。
- [[linux网络编程/函数笔记/IO多路复用/epoll_create|epoll_create]]：创建 epoll 实例，返回 epoll 文件描述符。
- [[linux网络编程/函数笔记/IO多路复用/epoll_ctl|epoll_ctl]]：向 epoll 实例添加、修改或删除关注的 fd。
- [[linux网络编程/函数笔记/IO多路复用/epoll_wait|epoll_wait]]：等待就绪事件，并把就绪事件写入用户提供的事件数组。
- [[linux网络编程/概念词条/epoll_event|epoll_event]]：描述关注事件和返回事件的数据结构，常用 `events` 和 `data.fd` 字段。
- [[linux网络编程/概念词条/epoll_data_t|epoll_data_t]]：`epoll_event.data` 的类型，常用 `fd` 成员保存文件描述符，也可以用 `ptr` 成员保存连接对象指针。

## 知识点补充

- `epoll` 将“维护监听集合”和“等待事件发生”拆开，避免每次都传入完整 fd 集合。
- 常见事件 `EPOLLIN` 表示可读。
- 默认常见工作方式是水平触发，边沿触发需要配合非阻塞 IO 更谨慎地处理。
- epoll 的 ET模式， 高效模式，但是只支持 非阻塞模式。 --- 忙轮询。
- `epoll_data_t` 是联合体，一次通常只用一个成员；课程代码最常用 `data.fd`。

## 本节内容速览

- `epoll_create` 创建 epoll 实例。
- `epoll_ctl` 注册监听 fd 和通信 fd。
- `epoll_wait` 等待就绪事件。
- 遍历返回的就绪事件数组，处理 `accept` 或读写。

## 复习时要回答

- 为什么 `epoll` 更适合高并发场景？
- `epoll_ctl` 和 `epoll_wait` 分别负责什么？
- `epoll_event.data.fd` 常用来保存什么？
- 水平触发和边沿触发有什么差别？

## 本节关键函数

- [[linux网络编程/函数笔记/IO多路复用/epoll_create|epoll_create]]
- [[linux网络编程/函数笔记/IO多路复用/epoll_ctl|epoll_ctl]]
- [[linux网络编程/函数笔记/IO多路复用/epoll_wait|epoll_wait]]

## 本节关键概念

- [[linux网络编程/概念词条/epoll模型|epoll模型]]
- [[linux网络编程/概念词条/select poll epoll对比|select、poll、epoll 对比]]
- [[linux网络编程/概念词条/Reactor反应堆模式|Reactor（反应堆模式）]]
- [[linux网络编程/概念词条/epoll_event|epoll_event]]
- [[linux网络编程/概念词条/epoll_data_t|epoll_data_t]]
- [[linux网络编程/概念词条/事件就绪|事件就绪]]
- [[linux网络编程/概念词条/水平触发与边沿触发|水平触发与边沿触发]]

## 关联模块

- [[linux网络编程/05 IO多路复用|05 IO多路复用]]
