---
title: HTTP 进阶
aliases:
  - HTTP八股
tags:
  - 八股
  - 网络
  - interview
  - network/http
status: active
created: 2026-06-09
---

# HTTP 进阶

返回：[[网络导航]]

相关：[[01 基础概念]] | [[04 安全与缓存]]

> [!abstract]
> 这页整理 HTTP/1.0、HTTP/1.1、HTTP/2、多 TCP 连接、连接池、队头阻塞、长连接等高频问题。

## 第一轮学习顺序

1. [[#HTTP/1.0 和 HTTP/1.1 有什么区别]]
2. [[#HTTP/2.0 有哪些改进]]
3. [[#为什么浏览器会建立多个 TCP 连接]]
4. [[#长连接和短连接有什么区别]]

## HTTP/1.0 和 HTTP/1.1 有什么区别

### 简要回答

HTTP/1.1 默认支持长连接，增加了 `Host` 头、分块传输、更完善的缓存、范围请求、更多状态码等能力。

### 对比表

| 对比点 | HTTP/1.0 | HTTP/1.1 |
|---|---|---|
| 连接 | 默认短连接 | 默认长连接 |
| Host 头 | 不强制 | 必须支持，便于虚拟主机 |
| 传输 | 主要依赖 `Content-Length` | 支持 `Transfer-Encoding: chunked` |
| 缓存 | `Expires`、`Last-Modified` 为主 | 增加 `Cache-Control`、`ETag` |
| 范围请求 | 支持不完善 | 支持 `Range` 和 `206` |
| 状态码 | 较少 | 增加 `100 Continue`、`409`、`410` 等 |
| 队头阻塞 | 存在 | 仍存在，管线化也没有彻底解决 |

### 示例

```http
# HTTP/1.1 默认长连接，也可以显式写 keep-alive
Connection: keep-alive
Keep-Alive: timeout=30

# Host 头：同一个 IP 上可以部署多个域名，服务器靠 Host 区分站点
Host: programmercarl.com

# 分块传输：响应体长度一开始不确定时，可以边生成边发送
Transfer-Encoding: chunked
```

```http
# HTTP/1.1 范围请求：只请求文件的一部分
Range: bytes=0-499

# 服务器返回部分内容
HTTP/1.1 206 Partial Content
Content-Range: bytes 0-499/1000
```

### 面试表达

> HTTP/1.1 相比 1.0 最大的改进是连接复用和协议能力增强，比如 Host、缓存控制、分块传输、范围请求。但 HTTP/1.1 仍有队头阻塞问题，所以后来 HTTP/2 引入多路复用。

## HTTP/2.0 有哪些改进

### 简要回答

HTTP/2.0 主要改进是二进制分帧、多路复用、头部压缩、流优先级、服务器推送。

### 详细回答

| 改进 | 作用 | 解决的问题 |
|---|---|---|
| 二进制分帧 | 把 HTTP 消息拆成二进制帧 | 解析更高效，便于多路复用 |
| 多路复用 | 一个 TCP 连接并发多个请求/响应 | 缓解 HTTP/1.1 应用层队头阻塞 |
| HPACK 头部压缩 | 压缩重复 Header | 减少 `User-Agent`、`Cookie` 等重复开销 |
| 流优先级 | 给不同资源设置优先级 | 优先加载关键 CSS/JS |
| 服务器推送 | 服务端主动推送资源 | 减少额外请求延迟 |

### 示例字段

```text
# HTTP/2 中可以重置某个流，而不是关闭整个连接
RST_STREAM

# 这些请求头经常重复，HTTP/2 通过 HPACK 降低重复传输成本
User-Agent
Cookie
Accept-Language
```

### 易错点

HTTP/2 解决的是 HTTP 应用层队头阻塞；由于底层常跑在 TCP 上，如果 TCP 丢包，仍可能影响同一 TCP 连接上的所有流。HTTP/3 基于 QUIC，进一步改善 TCP 层队头阻塞。

## 为什么浏览器会建立多个 TCP 连接

### 简要回答

在 HTTP/1.1 场景下，浏览器会对同一域名建立多个 TCP 连接并行加载资源，以提高页面加载速度并缓解队头阻塞。

### 详细回答

一个页面通常包含 HTML、CSS、JS、图片、字体等多个资源。如果只用一个 TCP 连接串行加载，后面的资源要等待前面的资源完成。浏览器建立多个连接后，可以并发下载多个资源。

```text
# 页面资源示例：浏览器可能并发请求这些资源
/index.html
/carl.js
/kama.css
/logo.png
```

### HTTP/1.1、HTTP/2、HTTP/3 对比

| 协议 | 并发方式 | 特点 |
|---|---|---|
| HTTP/1.1 | 多个 TCP 连接 + 连接池 | 绕开部分队头阻塞，但连接多 |
| HTTP/2 | 一个 TCP 连接上多路复用 | 减少连接数，但 TCP 丢包仍影响所有流 |
| HTTP/3 | QUIC 多路复用 | 基于 UDP，改善 TCP 层队头阻塞 |

### 后端补充

高并发服务器通常用 I/O 多路复用管理大量连接，例如 Linux 下常见 `epoll`。这部分和 [[../../linux网络编程/04 IO多路复用|Linux IO 多路复用]] 有关联。

```text
# epoll 常用于高并发网络服务器，让一个线程监听大量 socket 事件
epoll
```

## 长连接和短连接有什么区别

### 简要回答

短连接是一次请求响应后关闭 TCP 连接；长连接是多个请求复用同一个 TCP 连接。

### 对比表

| 对比点 | 短连接 | 长连接 |
|---|---|---|
| TCP 连接 | 每次请求新建连接 | 多次请求复用连接 |
| 开销 | 握手和挥手开销大 | 减少连接建立/关闭成本 |
| 资源占用 | 请求结束释放 | 空闲连接也占资源 |
| 适合场景 | 低频请求、简单场景 | Web 页面、多资源加载、频繁请求 |

### 示例

```http
# 复用 TCP 连接
Connection: keep-alive

# 请求后关闭 TCP 连接
Connection: close
```

### 面试表达

> 长连接减少了频繁三次握手和四次挥手的开销，但服务端需要管理空闲超时、最大请求数和连接资源。

## 资料来源

- [HTTP/1.0 与 HTTP/1.1 的区别](https://notes.kamacoder.com/base/http1-0-vs-1-1.html)
- [HTTP/2.0 与 HTTP/1.1 相比有哪些主要改进](https://notes.kamacoder.com/base/http2-improvements.html)
- [HTTP 是怎样实现多个 TCP 连接的](https://notes.kamacoder.com/base/http-multiple-tcp-connections.html)
