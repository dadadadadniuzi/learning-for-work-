---
title: epoll模型
tags:
  - linux
  - 网络编程
  - 概念词条
---
# epoll模型

## 它是什么

- epoll 是 Linux 提供的高效 IO 多路复用机制，适合大量 fd 监听场景。

## 工作流程

- `epoll_create` 创建 epoll 实例。
- `epoll_ctl` 添加、修改或删除要关注的 fd。
- `epoll_wait` 等待并取得就绪事件。

## 怎么理解

- `select/poll` 每次都把完整集合交给内核。
- `epoll` 把关注的 fd 保存在内核中，应用程序每次只需要等待就绪事件。
- `epoll_wait` 返回的是已经就绪的事件数组，减少无效扫描。

## 常见事件

- `EPOLLIN`：可读。
- `EPOLLOUT`：可写。
- `EPOLLERR`：错误。
- `EPOLLHUP`：挂起。
- `EPOLLET`：边沿触发。

## 易错点

- epoll 高效不代表代码一定简单。
- 边沿触发通常要配合非阻塞 IO，否则容易漏读或阻塞。

## 常见出现位置

- [[linux网络编程/课时笔记/05 IO多路复用/03 epoll|03 epoll]]
- [[linux网络编程/函数笔记/IO多路复用/epoll_create|epoll_create]]
