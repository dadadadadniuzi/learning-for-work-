---
title: epoll_create
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/IO多路复用
---
# epoll_create

> [!info] 功能
> 创建 epoll 实例，返回 epoll 文件描述符。

## 函数原型

- `int epoll_create(int size);`

## 依赖头文件

- `#include <sys/epoll.h>`

## 输入参数

- `size`：早期用于提示内核监听规模，现代 Linux 中大多已被忽略，但必须传大于 0 的值。课程中常见传 `1024` 等正数。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回 epoll 文件描述符。
- 失败返回 `-1`。

## 知识点补充

- 返回的 epoll fd 后续传给 `epoll_ctl` 和 `epoll_wait`。
- 使用完应通过 `close` 关闭。
- 新代码也常见 `epoll_create1`，但课程基础中通常讲 `epoll_create`。

## 易错点

- `epoll_create` 只是创建实例，不会自动监听任何 socket。
- 需要继续调用 [[linux网络编程/函数笔记/IO多路复用/epoll_ctl|epoll_ctl]] 注册 fd。

## 相关概念

- [[linux网络编程/概念词条/epoll模型|epoll模型]]

## 相关课时

- [[linux网络编程/课时笔记/05 IO多路复用/03 epoll|03 epoll]]

## 相关模块

- [[linux网络编程/05 IO多路复用|05 IO多路复用]]
