---
title: TCP 深入
aliases:
  - TCP八股
tags:
  - 八股
  - 网络
  - interview
  - network/tcp
status: active
created: 2026-06-09
---

# TCP 深入

返回：[[网络导航]]

相关：[[01 基础概念]] | [[../../linux网络编程/03 TCP通信与通信案例]]

> [!abstract]
> 这页整理三次握手、四次挥手、TIME_WAIT、可靠传输、拥塞控制、Keepalive 等 TCP 高频题。

## 第一轮学习顺序

1. [[#TCP 三次握手过程]]
2. [[#为什么 TCP 是三次握手，不是两次或四次]]
3. [[#TCP 四次挥手过程]]
4. [[#为什么会有 TIME_WAIT]]
5. [[#TCP 如何保证可靠传输]]
6. [[#TCP 拥塞控制有哪些机制]]
7. [[#TCP Keepalive 和 HTTP Keep-Alive 有什么区别]]

## TCP 三次握手过程

### 简要回答

三次握手是客户端和服务器建立 TCP 连接的过程：客户端发 `SYN`，服务器回 `SYN + ACK`，客户端再回 `ACK`。

### 详细流程

| 次数 | 发送方 | 报文 | 状态变化 | 含义 |
|---|---|---|---|---|
| 第 1 次 | 客户端 -> 服务器 | `SYN=1, seq=x` | 客户端进入 `SYN_SENT` | 客户端请求建立连接，并发送自己的初始序列号 |
| 第 2 次 | 服务器 -> 客户端 | `SYN=1, ACK=1, seq=y, ack=x+1` | 服务器进入 `SYN_RCVD` | 服务器同意连接，并确认客户端序列号 |
| 第 3 次 | 客户端 -> 服务器 | `ACK=1, ack=y+1` | 双方进入 `ESTABLISHED` | 客户端确认服务器序列号，连接建立 |

### 报文示意

```text
# 第一次：客户端告诉服务器“我想建立连接，我的初始序列号是 x”
Client -> Server: SYN = 1, seq = x

# 第二次：服务器告诉客户端“我收到了 x，我也想建立连接，我的初始序列号是 y”
Server -> Client: SYN = 1, ACK = 1, seq = y, ack = x + 1

# 第三次：客户端告诉服务器“我收到了 y”
Client -> Server: ACK = 1, ack = y + 1
```

### 面试表达

> 三次握手的本质是确认双方收发能力正常，并同步双方的初始序列号。不是单纯为了“打招呼”，而是为了后续可靠、有序传输做准备。

相关：[[../../linux网络编程/概念词条/TCP三次握手|TCP三次握手]]

## 为什么 TCP 是三次握手，不是两次或四次

### 简要回答

两次握手无法确认客户端能收到服务器的响应，也容易让历史 SYN 报文造成半开连接；四次握手可以，但多余，会增加延迟。

### 详细回答

- 两次不够：服务器回复 `SYN-ACK` 后，如果客户端没有收到或历史 `SYN` 报文突然到达，服务器可能误以为连接建立，浪费资源。
- 三次刚好：客户端和服务器都能确认自己的发送能力、接收能力，以及对方的发送能力、接收能力。
- 四次多余：第三次 ACK 已经能确认服务器的 SYN，无需再拆出额外一步。

### 易错点

- 三次握手不是为了传输 HTTP 数据，而是 TCP 连接建立阶段。
- `SYN` 报文会消耗服务器资源，所以大量伪造 SYN 可能导致 SYN Flood 攻击。
- 半开连接指一方以为连接存在，另一方并没有真正建立或已经断开。

## TCP 四次挥手过程

### 简要回答

四次挥手是 TCP 断开连接的过程：一方发 `FIN`，对方回 `ACK`；对方数据发完后再发 `FIN`，本端最后回 `ACK`。

### 详细流程

| 次数 | 发送方 | 报文 | 典型状态 | 含义 |
|---|---|---|---|---|
| 第 1 次 | 主动关闭方 -> 被动关闭方 | `FIN=1, seq=u` | `FIN_WAIT_1` | 主动方表示自己不再发送数据 |
| 第 2 次 | 被动关闭方 -> 主动关闭方 | `ACK=1, ack=u+1` | `CLOSE_WAIT` | 被动方确认收到 FIN，但可能还有数据没发完 |
| 第 3 次 | 被动关闭方 -> 主动关闭方 | `FIN=1, ACK=1, seq=w` | `LAST_ACK` | 被动方数据发完，也请求关闭 |
| 第 4 次 | 主动关闭方 -> 被动关闭方 | `ACK=1, ack=w+1` | `TIME_WAIT` | 主动方确认对方的 FIN |

### 报文示意

```text
# 第一次：主动关闭方说“我没有数据要发了”
Client -> Server: FIN = 1, seq = u

# 第二次：被动关闭方说“我知道你不发了，但我可能还要继续发”
Server -> Client: ACK = 1, ack = u + 1

# 第三次：被动关闭方说“我也发完了，可以关闭”
Server -> Client: FIN = 1, ACK = 1, seq = w

# 第四次：主动关闭方说“收到你的关闭请求”
Client -> Server: ACK = 1, ack = w + 1
```

### 为什么通常是四次

TCP 是全双工的，两个方向要分别关闭。收到对方 `FIN` 只能说明“对方不发了”，不代表自己也发完了，所以 ACK 和 FIN 往往分开发。

相关：[[../../linux网络编程/概念词条/TCP四次挥手|TCP四次挥手]]

## 为什么会有 TIME_WAIT

### 简要回答

TIME_WAIT 主要是为了确保最后一个 ACK 能被对方收到，并让旧连接中的延迟报文在网络中自然消失。

### 详细回答

TIME_WAIT 通常持续 2MSL：

- 如果最后 ACK 丢了，对方会重发 FIN，本端还能再次回复 ACK。
- 防止旧连接的延迟报文影响下一次相同四元组连接。
- 给网络中残留报文足够时间消失。

### TIME_WAIT 过多怎么办

| 问题 | 说明 | 常见处理 |
|---|---|---|
| 端口耗尽 | 主动关闭方大量进入 TIME_WAIT | 使用连接池、长连接、减少频繁短连接 |
| 服务端 TIME_WAIT 多 | 服务端主动关闭太多连接 | 检查超时策略，让客户端主动关闭 |
| 系统参数优化 | Linux 可调整部分 TCP 参数 | 如 `tcp_tw_reuse`，但要理解风险后再调 |

### 示例配置项

```text
# Linux 中与 TIME_WAIT 复用相关的参数
# 是否能开启要结合内核版本、业务场景和 NAT 环境判断
net.ipv4.tcp_tw_reuse
```

相关：[[../../linux网络编程/概念词条/TIME_WAIT|TIME_WAIT]]

## TCP 如何保证可靠传输

### 简要回答

TCP 通过序列号、确认应答、超时重传、快速重传、校验和、滑动窗口、流量控制、拥塞控制、连接管理来保证可靠传输。

### 机制表

| 机制 | 作用 | 解决什么问题 |
|---|---|---|
| 序列号 | 给字节编号 | 保证有序、去重 |
| ACK 确认 | 接收方确认已收到数据 | 判断是否丢包 |
| 超时重传 | 超时未收到 ACK 就重发 | 处理丢包 |
| 快速重传 | 收到多个重复 ACK 后提前重传 | 不等超时，降低延迟 |
| 校验和 | 检测传输错误 | 防止比特错误 |
| 滑动窗口 | 批量发送未确认数据 | 提升吞吐 |
| 流量控制 | 根据接收窗口限制发送 | 防止接收方来不及处理 |
| 拥塞控制 | 根据网络拥塞调整发送 | 防止网络被打爆 |

### 关键窗口

```text
# rwnd：接收窗口，接收方告诉发送方“我还能收多少”
rwnd

# cwnd：拥塞窗口，发送方根据网络拥塞程度自己维护
cwnd

# 实际可发送窗口通常受二者共同限制
EffectiveWindow = min(cwnd, rwnd)
```

### 面试表达

> TCP 的可靠性不是靠一个机制，而是一组机制配合：序列号保证顺序，ACK 确认收到，重传处理丢包，滑动窗口提升效率，流量控制保护接收方，拥塞控制保护网络。

## TCP 拥塞控制有哪些机制

### 简要回答

常见机制有慢启动、拥塞避免、快重传、快恢复。核心变量是拥塞窗口 `cwnd` 和慢启动阈值 `ssthresh`。

### 详细回答

| 机制 | 做什么 | 记忆点 |
|---|---|---|
| 慢启动 | `cwnd` 从小开始指数增长 | 先试探网络容量 |
| 拥塞避免 | 达到 `ssthresh` 后线性增长 | 增长变谨慎 |
| 快重传 | 收到多个重复 ACK 后立即重传 | 不等超时 |
| 快恢复 | 丢包后降低窗口，但不回到最初 | 避免吞吐骤降 |

### 关键变量

```text
# cwnd：拥塞窗口，越大表示发送方一次能发更多未确认数据
cwnd

# ssthresh：慢启动阈值，达到后从指数增长变为线性增长
ssthresh

# rwnd 是接收窗口，cwnd 是拥塞窗口，最终发送量要取更小的那个
EffectiveWindow = min(cwnd, rwnd)
```

### 拥塞控制和流量控制区别

| 对比 | 流量控制 | 拥塞控制 |
|---|---|---|
| 保护对象 | 接收方 | 整个网络 |
| 核心变量 | `rwnd` | `cwnd` |
| 目的 | 防止接收方缓冲区被撑爆 | 防止网络拥塞 |

### 补充

现代 TCP 还可能使用 CUBIC、BBR 等拥塞控制算法，但面试基础题先把慢启动、拥塞避免、快重传、快恢复讲清楚。

## TCP Keepalive 和 HTTP Keep-Alive 有什么区别

### 简要回答

TCP Keepalive 是传输层保活探测；HTTP Keep-Alive 是应用层复用 TCP 连接。

### 对比表

| 对比点 | TCP Keepalive | HTTP Keep-Alive |
|---|---|---|
| 所在层 | 传输层 TCP | 应用层 HTTP |
| 目的 | 检测空闲连接是否还活着 | 多个 HTTP 请求复用同一个 TCP 连接 |
| 管理者 | 操作系统内核 | Web 服务器、浏览器、应用 |
| 触发条件 | 连接长期空闲 | 多次 HTTP 请求 |
| 常见配置 | `SO_KEEPALIVE`、`net.ipv4.tcp_keepalive_*` | `Connection: keep-alive`、`KeepAliveTimeout` |

### 示例

```http
# HTTP/1.1 中复用连接，减少反复三次握手和四次挥手
Connection: keep-alive

# 显式关闭连接
Connection: close
```

```text
# Linux TCP Keepalive 相关配置项
net.ipv4.tcp_keepalive_*

# 程序中可通过 socket 选项开启 TCP Keepalive
SO_KEEPALIVE
```

### 面试表达

> TCP Keepalive 解决“连接是不是还活着”，HTTP Keep-Alive 解决“多个 HTTP 请求能不能复用同一条 TCP 连接”。名字像，但层次和目的不同。

## 资料来源

- [TCP 三次握手详解](https://notes.kamacoder.com/base/tcp-three-way-handshake.html)
- [三次握手的过程以及为什么是三次](https://notes.kamacoder.com/base/three-way-handshake-why.html)
- [四次挥手的过程是怎样的](https://notes.kamacoder.com/base/tcp-four-way-handshake.html)
- [TIME_WAIT 状态的作用](https://notes.kamacoder.com/base/time-wait-state.html)
- [TCP 连接如何确保可靠性](https://notes.kamacoder.com/base/tcp-reliability.html)
- [拥塞控制是怎样实现的](https://notes.kamacoder.com/base/congestion-control.html)
- [TCP Keepalive 和 HTTP Keep-Alive 的区别](https://notes.kamacoder.com/base/tcp-keepalive-vs-http-keep-alive.html)
