---
title: TIME_WAIT
tags:
  - linux
  - 网络编程
  - 概念词条
  - 网络编程/TCP通信
---
# TIME_WAIT

`TIME_WAIT` 是 TCP 主动关闭方在发送最后一个 `ACK` 后进入的等待状态。它通常出现在 [[linux网络编程/概念词条/TCP四次挥手|TCP四次挥手]] 的最后阶段。

## 为什么会有 TIME_WAIT

- 防止最后一个 `ACK` 丢失后无法响应对端重发的 `FIN`。
- 等待旧连接中的延迟报文过期，避免干扰后续相同四元组的新连接。

## 等多久

通常等待 [[linux网络编程/概念词条/MSL|2MSL]]，也就是两个最大报文生存时间。

## 谁会进入 TIME_WAIT

通常是主动关闭连接的一方。

例如客户端主动调用 `close`：

```text
ESTABLISHED -> FIN_WAIT_1 -> FIN_WAIT_2 -> TIME_WAIT -> CLOSED
```

如果服务器主动关闭连接，那么服务器也可能进入 `TIME_WAIT`。

## 易错点

- `TIME_WAIT` 是正常 TCP 状态，不是程序卡死。
- 长时间大量 `TIME_WAIT` 需要结合短连接数量、主动关闭方、端口复用等因素分析。
- `read/recv` 返回 `0` 的一方通常是被动关闭方，常见后续状态是 `CLOSE_WAIT`，不是 `TIME_WAIT`。

## 相关概念

- [[linux网络编程/概念词条/MSL|MSL]]
- [[linux网络编程/概念词条/TCP状态转换图|TCP状态转换图]]
- [[linux网络编程/概念词条/TCP四次挥手|TCP四次挥手]]

## 相关模块

- [[linux网络编程/03 TCP通信与通信案例|03 TCP通信与通信案例]]
