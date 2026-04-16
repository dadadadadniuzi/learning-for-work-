---
title: BEV_OPT_CLOSE_ON_FREE
tags:
  - linux
  - 网络编程
  - 概念词条
---
# BEV_OPT_CLOSE_ON_FREE

## 它是什么

- `BEV_OPT_CLOSE_ON_FREE` 是创建 bufferevent 时常用的选项。
- 它表示释放 bufferevent 时，同时关闭其关联的底层 socket fd。

## 怎么理解

- 如果没有这个选项，释放 bufferevent 不一定关闭 fd。
- 使用它可以减少忘记关闭通信 fd 的风险。

## 常见写法

```c
bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
```

## 易错点

- 使用该选项后，不要再重复手动关闭同一个 fd。
- 它影响资源释放行为，不影响是否启用读写事件。

## 常见出现位置

- [[linux网络编程/函数笔记/Libevent/bufferevent_socket_new|bufferevent_socket_new]]
