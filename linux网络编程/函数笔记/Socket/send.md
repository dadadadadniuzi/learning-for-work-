---
title: send
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/Socket
---
# send

> [!info] 功能
> 向已连接 socket 发送数据，常用于 TCP 通信。

## 函数原型

- `ssize_t send(int sockfd, const void *buf, size_t len, int flags);`

## 依赖头文件

- `#include <sys/types.h>`
- `#include <sys/socket.h>`

## 输入参数

- `sockfd`：已连接 socket 文件描述符。客户端通常是 `connect` 成功后的 socket；服务器端通常是 `accept` 返回的 `cfd`。
- `buf`：待发送数据缓冲区首地址。它是 `const void *`，说明 `send` 只读取这块内存，不会修改缓冲区内容。
- `len`：希望发送的字节数，类型是 [[linux网络编程/概念词条/size_t|size_t]]。它不是字符串长度的自动计算结果，如果发送字符串，通常要自己决定是否包含结尾 `\0`。
- `flags`：发送控制标志。基础阶段通常传 `0`，表示按默认方式发送；扩展标志见 [[linux网络编程/概念词条/send-recv标志|send-recv标志]]。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回实际发送的字节数，类型是 [[linux网络编程/概念词条/ssize_t|ssize_t]]。
- 返回值可能小于 `len`，这叫短写，需要根据场景继续发送剩余数据。
- 失败返回 `-1`，并设置错误信息。

## 知识点补充

- 对 TCP 来说，`send` 发送的是字节流，不保留应用层消息边界。
- `send` 成功只表示数据进入本机内核发送缓冲区，不等于对端应用已经处理完。
- 如果使用 `flags = 0`，在很多简单 TCP 场景下可以把它和 `write` 类比理解。

## 常见用法

```c
send(cfd, buf, len, 0);
```

## 易错点

- 不要默认一次 `send` 一定发送完所有数据。
- 不要默认对端一次 `recv` 就能完整收到一次 `send` 的全部内容。
- 如果对端已经关闭连接，继续发送可能失败。

## 相关概念

- [[linux网络编程/概念词条/TCP|TCP]]
- [[linux网络编程/概念词条/已连接套接字|已连接套接字]]
- [[linux网络编程/概念词条/send-recv标志|send-recv标志]]
- [[linux网络编程/概念词条/size_t|size_t]]
- [[linux网络编程/概念词条/ssize_t|ssize_t]]

## 相关课时

- [[linux网络编程/课时笔记/03 TCP通信与通信案例/01 TCP通信基础案例|01 TCP通信基础案例]]
- [[linux网络编程/课时笔记/03 TCP通信与通信案例/02 客户端与服务器通信流程|02 客户端与服务器通信流程]]

## 相关模块

- [[linux网络编程/03 TCP通信与通信案例|03 TCP通信与通信案例]]
