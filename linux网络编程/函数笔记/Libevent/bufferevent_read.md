---
title: bufferevent_read
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/Libevent
---
# bufferevent_read

> [!info] 功能
> 从 [[linux网络编程/概念词条/bufferevent|bufferevent]] 的输入缓冲区读取数据到用户缓冲区。

## 函数原型

- `size_t bufferevent_read(struct bufferevent *bufev, void *data, size_t size);`

## 依赖头文件

- `#include <event2/bufferevent.h>`

## 输入参数

- `bufev`：目标 [[linux网络编程/概念词条/bufferevent|bufferevent]]。
- `data`：用户提供的接收缓冲区首地址，用来保存读出的数据。
- `size`：最多读取的字节数，类型是 [[linux网络编程/概念词条/size_t|size_t]]。它不能超过 `data` 指向缓冲区的真实容量。

## 输出参数

- `data`：函数会把从输入缓冲区取出的数据写入这里。它不保证自动补 `\0`，当作字符串使用时要自己处理结尾。

## 返回值

- 返回实际读取的字节数，可能小于 `size`。

## 知识点补充

- 这个函数读的是 [[linux网络编程/概念词条/bufferevent读写缓冲区|bufferevent输入缓冲区]]，不是直接调用系统 `read`。
- 读回调触发时，通常说明输入缓冲区已经有数据可取。
- 如果协议按行或按固定长度解析，需要自己处理粘包、半包等应用层边界问题。

## 常见用法

```c
char buf[1024] = {0};
size_t n = bufferevent_read(bev, buf, sizeof(buf) - 1);
```

## 易错点

- 返回值是 `size_t`，不是负数错误码。
- 缓冲区内容不一定是 C 字符串，打印前要确认是否补了字符串结束符。

## 相关概念

- [[linux网络编程/概念词条/bufferevent|bufferevent]]
- [[linux网络编程/概念词条/bufferevent读写缓冲区|bufferevent读写缓冲区]]
- [[linux网络编程/概念词条/size_t|size_t]]

## 相关课时

- [[linux网络编程/课时笔记/07 Libevent库/05 bufferevent基础|05 bufferevent基础]]

## 相关模块

- [[linux网络编程/07 Libevent库|07 Libevent库]]
