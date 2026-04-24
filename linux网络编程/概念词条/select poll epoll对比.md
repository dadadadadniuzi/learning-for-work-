---
title: select poll epoll对比
tags:
  - linux
  - 网络编程
  - 概念词条
  - 网络编程/IO多路复用
---
# select poll epoll对比

## 它解决什么问题

[[linux网络编程/概念词条/select模型|select]]、[[linux网络编程/概念词条/poll模型|poll]]、[[linux网络编程/概念词条/epoll模型|epoll]] 都属于 [[linux网络编程/概念词条/IO多路复用|IO多路复用]]：让一个进程或线程同时监听多个 fd，哪个 fd 就绪就处理哪个。

它们的核心区别在于：

- 用什么数据结构告诉内核“我要监听哪些 fd”。
- 内核如何返回“哪些 fd 已经就绪”。
- 每次调用是否要重复传递全部 fd。
- 应用层是否还需要线性遍历。

## 总体对比

| 对比项     | select                            | poll                    | epoll                                               |
| ------- | --------------------------------- | ----------------------- | --------------------------------------------------- |
| 核心函数    | [[select]]                        | [[poll]]                | [[epoll_create]]<br>[[epoll_ctl]]<br>[[epoll_wait]] |
| 监听集合    | [[linux网络编程/概念词条/fd_set\|fd_set]] | [[pollfd]]              | 内核维护 epoll 实例，用户用 [[epoll_event]]<br>注册和接收事件        |
| 传参方式    | 每次调用都传入 fd 集合                     | 每次调用都传入 pollfd 数组       | 先用 epoll_ctl 注册，之后 epoll_wait 等待                    |
| 返回方式    | 修改传入的 fd_set，只保留就绪 fd             | 修改每个 pollfd 的 revents   | 把就绪事件写入 events 数组                                   |
| fd 数量限制 | 受 [[FD_SETSIZE]]<br>影响<br>        | 不受 fd_set 位图限制，但数组由用户维护 | 适合大量连接，限制更多来自系统资源                                   |
| 是否需要遍历  | 需要遍历 fd 范围                        | 需要遍历 pollfd 数组          | 只遍历返回的就绪事件                                          |
| 常见适用场景  | 少量 fd、兼容性要求高                      | 中等规模、想避免 fd_set 限制      | 高并发、大量连接                                            |

## select 怎么工作

[[linux网络编程/函数笔记/IO多路复用/select|select]] 使用 [[linux网络编程/概念词条/fd_set|fd_set]] 集合。

调用前：

- 用 [[linux网络编程/函数笔记/IO多路复用/FD_ZERO|FD_ZERO]] 清空集合。
- 用 [[linux网络编程/函数笔记/IO多路复用/FD_SET|FD_SET]] 把 fd 加入集合。
- `nfds` 传最大 fd 加 1。

调用后：

- 内核会改写传入的 [[linux网络编程/概念词条/fd_set|fd_set]]。
- 应用程序用 [[linux网络编程/函数笔记/IO多路复用/FD_ISSET|FD_ISSET]] 判断哪个 fd 就绪。

关键问题：

- [[linux网络编程/概念词条/fd_set|fd_set]] 有容量限制。
- 每轮调用前通常要重新复制集合。
- 返回后仍要遍历 fd 范围。

## poll 怎么工作

[[linux网络编程/函数笔记/IO多路复用/poll|poll]] 使用 [[linux网络编程/概念词条/pollfd|struct pollfd]] 数组。

每个元素包含：

- `fd`：要监听的文件描述符。
- `events`：应用程序关心的事件，例如 [[linux网络编程/概念词条/poll事件宏|POLLIN]]。
- `revents`：内核返回的实际就绪事件。

相比 select：

- 不需要最大 fd 加 1。
- 不受 [[linux网络编程/概念词条/FD_SETSIZE|FD_SETSIZE]] 位图限制。
- `events` 和 `revents` 分开，比 select 改写原集合更直观。

关键问题：

- 每次调用仍要把数组交给内核。
- 返回后仍要线性遍历数组。

## epoll 怎么工作

[[linux网络编程/概念词条/epoll模型|epoll]] 把“注册关注 fd”和“等待就绪事件”拆开。

典型流程：

```text
epoll_create 创建 epoll 实例
epoll_ctl 注册、修改、删除 fd
epoll_wait 等待就绪事件
```

注册时使用 [[linux网络编程/概念词条/epoll_event|epoll_event]]：

- `events` 保存关注事件，例如 `EPOLLIN`。
- `data` 类型是 [[linux网络编程/概念词条/epoll_data_t|epoll_data_t]]，常用 `data.fd` 保存 fd。

相比 select/poll：

- 不需要每次传入全部监听 fd。
- 内核维护关注集合。
- [[linux网络编程/函数笔记/IO多路复用/epoll_wait|epoll_wait]] 返回的是就绪事件数组，应用程序只遍历就绪事件。

## 记忆口诀

- select：位图集合，每次传入，每次改写，回来遍历。
- poll：结构体数组，events/revents 分离，回来仍遍历。
- epoll：先注册，再等待，只处理返回的就绪事件。

## 怎么选择

- 少量连接、学习基础模型：先掌握 [[linux网络编程/函数笔记/IO多路复用/select|select]]。
- 想理解数组式事件管理：学习 [[linux网络编程/函数笔记/IO多路复用/poll|poll]]。
- 高并发服务器重点：掌握 [[linux网络编程/概念词条/epoll模型|epoll模型]]。

实际 Linux 高并发网络服务器中，通常更偏向使用 epoll 或基于 epoll 封装的事件库，例如 [[linux网络编程/概念词条/Libevent|Libevent]]。

## 易错点

- [[linux网络编程/函数笔记/IO多路复用/select|select]] 的 `nfds` 是最大 fd 加 1；[[linux网络编程/函数笔记/IO多路复用/poll|poll]] 的 [[linux网络编程/概念词条/nfds_t|nfds_t]] 参数是数组元素数量。
- select 和 poll 返回后都不是“自动处理连接”，仍然需要应用程序判断并调用 [[linux网络编程/函数笔记/Socket/accept|accept]]、[[linux网络编程/函数笔记/Socket/read|read]] 或 [[linux网络编程/函数笔记/Socket/recv|recv]]。
- epoll 也不是不用遍历，而是只遍历这次返回的就绪事件。
- [[linux网络编程/概念词条/水平触发与边沿触发|边沿触发]] 不是默认最安全模式，使用时通常要配合非阻塞 IO。

## 相关函数

- [[linux网络编程/函数笔记/IO多路复用/select|select]]
- [[linux网络编程/函数笔记/IO多路复用/poll|poll]]
- [[linux网络编程/函数笔记/IO多路复用/epoll_create|epoll_create]]
- [[linux网络编程/函数笔记/IO多路复用/epoll_ctl|epoll_ctl]]
- [[linux网络编程/函数笔记/IO多路复用/epoll_wait|epoll_wait]]

## 相关概念

- [[linux网络编程/概念词条/IO多路复用|IO多路复用]]
- [[linux网络编程/概念词条/select模型|select模型]]
- [[linux网络编程/概念词条/poll模型|poll模型]]
- [[linux网络编程/概念词条/epoll模型|epoll模型]]
- [[linux网络编程/概念词条/fd_set|fd_set]]
- [[linux网络编程/概念词条/pollfd|struct pollfd]]
- [[linux网络编程/概念词条/epoll_event|epoll_event]]
- [[linux网络编程/概念词条/epoll_data_t|epoll_data_t]]

## 相关课时

- [[linux网络编程/课时笔记/05 IO多路复用/01 select|01 select]]
- [[linux网络编程/课时笔记/05 IO多路复用/02 poll|02 poll]]
- [[linux网络编程/课时笔记/05 IO多路复用/03 epoll|03 epoll]]

## 相关模块

- [[linux网络编程/05 IO多路复用|05 IO多路复用]]
