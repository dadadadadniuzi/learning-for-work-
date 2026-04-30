---
title: SO_LINGER
tags:
  - linux
  - 网络编程
  - 概念词条
---
# SO_LINGER

## 它是什么

`SO_LINGER` 是一个 socket 选项，用来控制“关闭 socket 时，内核对剩余待发送数据怎么处理”。

它通常配合 [[linux网络编程/函数笔记/Socket/setsockopt|setsockopt]] 使用，所属层级是 [[linux网络编程/概念词条/SOL_SOCKET|SOL_SOCKET]]。

## 依赖结构

- [[linux网络编程/概念词条/struct linger|struct linger]]

## 常见用法

```c
struct linger tmp;
tmp.l_onoff = 1;
tmp.l_linger = 5;

setsockopt(fd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
```

这表示：

- 开启 `SO_LINGER`
- 调用 `close` 时最多等待 `5` 秒处理剩余数据

## 怎么理解

默认情况下，`close` 往往是“把关闭动作交给内核继续处理”，应用程序很快返回。

而 `SO_LINGER` 可以改变这种行为，让关闭连接时变成下面几种模式之一：

### 1. 不启用 linger

- `l_onoff = 0`
- 使用系统默认行为
- `close` 一般很快返回

### 2. 启用 linger，并设置等待时间大于 0

- `l_onoff = 1`
- `l_linger > 0`
- `close` 可能阻塞一段时间，等待待发送数据尽量发完

### 3. 启用 linger，并设置等待时间为 0

- `l_onoff = 1`
- `l_linger = 0`
- 常被理解为“立即丢弃未发送完的数据并快速复位连接”
- 这种情况通常会让对端更像收到一个 `RST`

## 在网络编程里有什么用

- 控制 `close` 时是否等待剩余数据发完
- 调试连接关闭行为
- 某些需要快速中断连接的场景会用到

## 易错点

- `SO_LINGER` 不是“提高性能”的常规优化项，平时并不需要默认去开。
- 如果设置成“等待发送完成”，`close` 可能阻塞，非阻塞模型里要格外小心。
- 如果设置成 `l_linger = 0`，可能导致未发完的数据被直接丢弃。
- 它控制的是“关闭阶段”的行为，不是正常收发阶段。

## 和相关概念的关系

- [[linux网络编程/概念词条/SOL_SOCKET|SOL_SOCKET]]：说明 `SO_LINGER` 属于 socket 层选项
- [[linux网络编程/函数笔记/Socket/setsockopt|setsockopt]]：设置 `SO_LINGER` 的函数
- [[linux网络编程/概念词条/TCP半关闭|TCP半关闭]]：半关闭和 linger 不是同一个概念
- [[linux网络编程/概念词条/TIME_WAIT|TIME_WAIT]]：`SO_LINGER` 也不等于 `TIME_WAIT` 控制

## 常见出现位置

- [[linux网络编程/函数笔记/Socket/setsockopt|setsockopt]]
- [[linux网络编程/函数笔记/Socket/getsockopt|getsockopt]]

