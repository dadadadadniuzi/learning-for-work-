---
title: fd_set
tags:
  - linux
  - 网络编程
  - 概念词条
  - 网络编程/IO多路复用
---
# fd_set

## 它是什么

[[linux网络编程/概念词条/fd_set|fd_set]] 是 [[linux网络编程/函数笔记/IO多路复用/select|select]] 使用的“文件描述符集合”类型。

它用来告诉内核：我希望你帮我观察这些文件描述符，看看哪些已经满足读、写或异常条件。

## 依赖头文件

```c
#include <sys/select.h>
```

部分系统也会在这些头文件中暴露相关定义：

```c
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
```

## 怎么理解

可以把 [[linux网络编程/概念词条/fd_set|fd_set]] 理解成一个位图。

- 文件描述符 `0` 对应第 0 位。
- 文件描述符 `1` 对应第 1 位。
- 某一位为 1，表示对应 fd 在集合中。
- 某一位为 0，表示对应 fd 不在集合中。

例如把监听套接字 `lfd` 和客户端套接字 `cfd` 加入读集合，就是告诉 [[linux网络编程/函数笔记/IO多路复用/select|select]]：请帮我观察它们是否“可读”。

## 在 select 中的位置

[[linux网络编程/函数笔记/IO多路复用/select|select]] 原型中有三个 [[linux网络编程/概念词条/fd_set|fd_set]] 指针参数：

```c
int select(int nfds,
           fd_set *readfds,
           fd_set *writefds,
           fd_set *exceptfds,
           struct timeval *timeout);
```

它们分别表示：

- `readfds`：读集合，监听哪些 fd 是否可读。
- `writefds`：写集合，监听哪些 fd 是否可写。
- `exceptfds`：异常集合，监听哪些 fd 是否出现异常状态。

如果某类事件不关心，可以传 `NULL`。

## 输入和输出变化

[[linux网络编程/概念词条/fd_set|fd_set]] 在 [[linux网络编程/函数笔记/IO多路复用/select|select]] 中既是输入参数，也是输出参数。

调用前：

- 集合中保存“希望监听的 fd”。

调用后：

- 集合会被内核改写。
- 只保留“已经就绪的 fd”。
- 没就绪的 fd 会从集合中被清掉。

这就是为什么循环服务器里通常要准备两个集合：

- `allset`：保存所有需要长期监听的 fd。
- `rset`：每轮调用 [[linux网络编程/函数笔记/IO多路复用/select|select]] 前从 `allset` 复制一份，交给内核改写。

## 常见操作宏

- [[linux网络编程/函数笔记/IO多路复用/FD_ZERO|FD_ZERO]]：清空集合，把所有位都置 0。
- [[linux网络编程/函数笔记/IO多路复用/FD_SET|FD_SET]]：把某个 fd 加入集合。
- [[linux网络编程/函数笔记/IO多路复用/FD_CLR|FD_CLR]]：把某个 fd 从集合中移除。
- [[linux网络编程/函数笔记/IO多路复用/FD_ISSET|FD_ISSET]]：判断某个 fd 是否仍在集合里，常用于判断 [[linux网络编程/函数笔记/IO多路复用/select|select]] 返回后哪个 fd 就绪。

## 常见代码

```c
fd_set allset;
fd_set rset;

FD_ZERO(&allset);
FD_SET(lfd, &allset);

while (1) {
    rset = allset;

    int nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
    if (nready == -1) {
        perror("select");
        break;
    }

    if (FD_ISSET(lfd, &rset)) {
        int cfd = accept(lfd, NULL, NULL);
        FD_SET(cfd, &allset);
    }
}
```

## FD_SETSIZE 限制

[[linux网络编程/概念词条/fd_set|fd_set]] 通常有固定大小限制，这个限制常由 [[linux网络编程/概念词条/FD_SETSIZE|FD_SETSIZE]] 决定。

如果 fd 数量很多，[[linux网络编程/概念词条/select模型|select模型]] 就会暴露两个问题：

- 集合大小有限，不能无限加入 fd。
- 每次返回后应用层还要遍历 fd 判断谁就绪，连接很多时效率较低。

这也是后来 [[linux网络编程/概念词条/poll模型|poll模型]] 和 [[linux网络编程/概念词条/epoll模型|epoll模型]] 被引入的重要原因。

## 易错点

- [[linux网络编程/函数笔记/IO多路复用/select|select]] 会修改传入的 [[linux网络编程/概念词条/fd_set|fd_set]]，循环里不要直接反复使用被改写后的集合。
- [[linux网络编程/函数笔记/IO多路复用/FD_SET|FD_SET]]、[[linux网络编程/函数笔记/IO多路复用/FD_CLR|FD_CLR]] 等宏参数通常要传集合地址，例如 `&allset`。
- `nfds` 参数不是 fd 个数，而是当前监听的最大 fd 加 1。
- 监听 socket 可读通常表示有新连接可以 [[linux网络编程/函数笔记/Socket/accept|accept]]；通信 socket 可读通常表示有数据可读，或者对端关闭。

## 相关函数

- [[linux网络编程/函数笔记/IO多路复用/select|select]]
- [[linux网络编程/函数笔记/IO多路复用/FD_ZERO|FD_ZERO]]
- [[linux网络编程/函数笔记/IO多路复用/FD_SET|FD_SET]]
- [[linux网络编程/函数笔记/IO多路复用/FD_CLR|FD_CLR]]
- [[linux网络编程/函数笔记/IO多路复用/FD_ISSET|FD_ISSET]]

## 相关概念

- [[linux网络编程/概念词条/select模型|select模型]]
- [[linux网络编程/概念词条/IO多路复用|IO多路复用]]
- [[linux网络编程/概念词条/事件就绪|事件就绪]]
- [[linux网络编程/概念词条/FD_SETSIZE|FD_SETSIZE]]
- [[linux网络编程/概念词条/pollfd|pollfd]]

## 相关课时

- [[linux网络编程/课时笔记/05 IO多路复用/01 select|01 select]]

## 相关模块

- [[linux网络编程/05 IO多路复用|05 IO多路复用]]
