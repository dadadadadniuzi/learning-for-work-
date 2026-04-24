---
title: select
tags:
  - linux
  - 网络编程
  - 课时笔记
  - 网络编程/IO多路复用
---
# select

## 本节学什么

- [[linux网络编程/概念词条/select模型|select模型]]
- [[linux网络编程/概念词条/fd_set|fd_set]]
- [[linux网络编程/概念词条/select poll epoll对比|select、poll、epoll 对比]]
- [[linux网络编程/函数笔记/IO多路复用/FD_ZERO|FD_ZERO]]、[[linux网络编程/函数笔记/IO多路复用/FD_SET|FD_SET]]、[[linux网络编程/函数笔记/IO多路复用/FD_CLR|FD_CLR]]、[[linux网络编程/函数笔记/IO多路复用/FD_ISSET|FD_ISSET]]
- [[linux网络编程/函数笔记/IO多路复用/select|select]] 的监听集合和返回值
- [[linux网络编程/概念词条/FD_SETSIZE|FD_SETSIZE]] 限制
- [[linux网络编程/概念词条/struct timeval|struct timeval]] 超时参数

## 本节学什么详解

- [[linux网络编程/概念词条/select模型|select模型]]：应用程序把多个 fd 放进集合，调用 [[linux网络编程/函数笔记/IO多路复用/select|select]] 让内核检查哪些 fd 已经满足读、写或异常条件。
- [[linux网络编程/概念词条/fd_set|fd_set]]：[[linux网络编程/函数笔记/IO多路复用/select|select]] 使用的文件描述符集合，本质上可以理解成位图，一个 fd 对应集合中的某一位。
- [[linux网络编程/概念词条/select poll epoll对比|select、poll、epoll 对比]]：从监听集合、返回方式、遍历成本和适用场景横向比较三种 IO 多路复用模型。
- [[linux网络编程/函数笔记/IO多路复用/FD_ZERO|FD_ZERO]]、[[linux网络编程/函数笔记/IO多路复用/FD_SET|FD_SET]]、[[linux网络编程/函数笔记/IO多路复用/FD_CLR|FD_CLR]]、[[linux网络编程/函数笔记/IO多路复用/FD_ISSET|FD_ISSET]]：分别用于清空集合、加入 fd、移除 fd、判断 fd 是否在集合中。
- [[linux网络编程/函数笔记/IO多路复用/select|select]] 返回值：大于 0 表示就绪 fd 总数，等于 0 表示超时，等于 -1 表示出错。
- [[linux网络编程/概念词条/FD_SETSIZE|FD_SETSIZE]]：[[linux网络编程/概念词条/fd_set|fd_set]] 的容量限制，说明 [[linux网络编程/概念词条/select模型|select模型]] 不适合无限数量连接。
- [[linux网络编程/概念词条/struct timeval|struct timeval]]：控制 [[linux网络编程/函数笔记/IO多路复用/select|select]] 最多阻塞多久。传 `NULL` 表示一直阻塞，传 `{0,0}` 表示立即返回，传正时间表示超时等待。

## 知识点补充

- [[linux网络编程/函数笔记/IO多路复用/select|select]] 会修改传入的 [[linux网络编程/概念词条/fd_set|fd_set]] 集合，所以通常要维护 `allset`，每轮循环复制给 `rset`。
- `nfds` 参数要传“最大 fd + 1”。
- 读集合中如果监听 socket 就绪，说明可以 `accept`；如果通信 socket 就绪，说明可以 `recv/read`。

## 本节内容速览

- 建立监听 socket。
- 把监听 fd 放入 `allset`。
- 循环复制集合并调用 [[linux网络编程/函数笔记/IO多路复用/select|select]]。
- 根据 [[linux网络编程/函数笔记/IO多路复用/FD_ISSET|FD_ISSET]] 判断哪个 fd 就绪。
- 对监听 fd 执行 `accept`，对通信 fd 执行读写。

## 复习时要回答

- `select` 如何配合监听集合工作？
- 为什么每轮循环前要重新设置或复制 fd 集合？
- `nfds` 为什么是最大 fd 加 1？
- 监听 fd 就绪和通信 fd 就绪分别表示什么？

## 本节关键函数

- [[linux网络编程/函数笔记/IO多路复用/select|select]]
- [[linux网络编程/函数笔记/IO多路复用/FD_ZERO|FD_ZERO]]
- [[linux网络编程/函数笔记/IO多路复用/FD_SET|FD_SET]]
- [[linux网络编程/函数笔记/IO多路复用/FD_CLR|FD_CLR]]
- [[linux网络编程/函数笔记/IO多路复用/FD_ISSET|FD_ISSET]]

## 本节关键概念

- [[linux网络编程/概念词条/select模型|select模型]]
- [[linux网络编程/概念词条/fd_set|fd_set]]
- [[linux网络编程/概念词条/select poll epoll对比|select、poll、epoll 对比]]
- [[linux网络编程/概念词条/事件就绪|事件就绪]]
- [[linux网络编程/概念词条/FD_SETSIZE|FD_SETSIZE]]
- [[linux网络编程/概念词条/struct timeval|struct timeval]]

## 关联模块

- [[linux网络编程/05 IO多路复用|05 IO多路复用]]
