---
title: bufferevent_write
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/Libevent
---
# bufferevent_write

> [!info] 功能
> 把用户数据写入 [[linux网络编程/概念词条/bufferevent|bufferevent]] 的输出缓冲区，由 Libevent 异步发送到对端。

## 函数原型

- `int bufferevent_write(struct bufferevent *bufev, const void *data, size_t size);`

## 依赖头文件

- `#include <event2/bufferevent.h>`

## 输入参数

- `bufev`：目标 [[linux网络编程/概念词条/bufferevent|bufferevent]]。
- `data`：要发送的数据首地址，可以是字符串、结构体序列化后的字节流或其他二进制数据。
- `size`：要写入输出缓冲区的字节数，类型是 [[linux网络编程/概念词条/size_t|size_t]]。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回 `0`。
- 失败返回 `-1`。

## 知识点补充

- 这个函数通常不会立刻把数据全部发到网卡，而是先进入 [[linux网络编程/概念词条/bufferevent读写缓冲区|bufferevent输出缓冲区]]。
- 后续真正的发送由事件循环驱动，因此它适合高并发异步通信。
- 如果传字符串，`size` 是否包含结尾 `\0` 由协议自己决定。

## 常见用法

```c
bufferevent_write(bev, "hello\n", 6);
```

## 易错点

- 不要用 `strlen` 发送二进制数据，因为二进制数据中可能包含 `\0`。
- 返回 `0` 只表示成功放入缓冲区，不代表对端已经收到。

## 相关概念

- [[linux网络编程/概念词条/bufferevent|bufferevent]]
- [[linux网络编程/概念词条/bufferevent读写缓冲区|bufferevent读写缓冲区]]
- [[linux网络编程/概念词条/size_t|size_t]]

## 相关课时

- [[linux网络编程/课时笔记/07 Libevent库/05 bufferevent基础|05 bufferevent基础]]

## 相关模块

- [[linux网络编程/07 Libevent库|07 Libevent库]]
