---
title: pollfd
tags:
  - linux
  - 网络编程
  - 概念词条
---
# pollfd

## 它是什么

- `struct pollfd` 是 `poll` 使用的监听项结构。
- 一个 `pollfd` 元素描述一个 fd 以及它关心和实际发生的事件。

## 常见原型

```c
struct pollfd {
    int   fd;
    short events;
    short revents;
};
```

## 字段说明

- `fd`：要监听的文件描述符。
- `events`：应用程序关心的事件，例如 `POLLIN` 表示可读。
- `revents`：内核返回的实际就绪事件。

## 怎么理解

- `poll` 不使用 `fd_set` 位图，而是使用 `pollfd` 数组。
- 应用程序维护一个数组，数组中每个有效元素代表一个被监听的 fd。

## 易错点

- `events` 是调用前设置的，`revents` 是调用后检查的。
- 不再监听某个数组元素时，常把 `fd` 设为 `-1`。

## 常见出现位置

- [[linux网络编程/函数笔记/IO多路复用/poll|poll]]
- [[linux网络编程/课时笔记/05 IO多路复用/02 poll|02 poll]]
