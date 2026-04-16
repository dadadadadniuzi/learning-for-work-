---
title: fd_set
tags:
  - linux
  - 网络编程
  - 概念词条
---
# fd_set

## 它是什么

- `fd_set` 是 `select` 使用的文件描述符集合类型。
- 它保存一组要监听的 fd，常用于读集合、写集合、异常集合。

## 怎么理解

- 可以把 `fd_set` 理解成位图。
- 某个 fd 在集合里，就表示对应位置被标记。
- `select` 返回后，集合会被内核改写，只保留已经就绪的 fd。

## 常见操作

- [[linux网络编程/函数笔记/IO多路复用/FD_ZERO|FD_ZERO]]：清空集合。
- [[linux网络编程/函数笔记/IO多路复用/FD_SET|FD_SET]]：把 fd 加入集合。
- [[linux网络编程/函数笔记/IO多路复用/FD_CLR|FD_CLR]]：把 fd 从集合移除。
- [[linux网络编程/函数笔记/IO多路复用/FD_ISSET|FD_ISSET]]：判断 fd 是否在集合中。

## 易错点

- `select` 会修改传入的 `fd_set`，所以循环中通常需要备份集合。
- `fd_set` 有大小限制，通常受 `FD_SETSIZE` 影响。

## 常见出现位置

- [[linux网络编程/函数笔记/IO多路复用/select|select]]
- [[linux网络编程/课时笔记/05 IO多路复用/01 select|01 select]]
