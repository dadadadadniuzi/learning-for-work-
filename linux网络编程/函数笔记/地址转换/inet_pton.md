---
title: inet_pton
tags:
  - linux
  - 网络编程
  - 函数卡片
  - 网络编程/地址转换
---
# inet_pton

> [!info] 功能
> 将文本形式的 IP 地址转换成网络字节序的二进制地址。

## 函数原型

- `int inet_pton(int af, const char *src, void *dst);`

## 依赖头文件

- `#include <arpa/inet.h>`

## 输入参数

- `af`：地址族，决定按哪种 IP 地址格式解析 `src`。IPv4 使用 `AF_INET`，IPv6 使用 `AF_INET6`。
- `src`：输入的字符串 IP 地址。IPv4 常见格式如 `"127.0.0.1"`、`"192.168.1.10"`。它必须是合法 IP 字符串，不能是域名。
- `dst`：输出缓冲区地址，用来接收转换后的网络字节序二进制地址。IPv4 场景通常传 `&addr.sin_addr.s_addr` 或 `&addr.sin_addr`，必须保证指向的空间足够保存对应地址族的数据。

## 输出参数

- `dst`：成功时被写入转换后的二进制网络地址。

## 返回值

- 成功返回 `1`。
- `src` 格式不合法返回 `0`。
- 出错返回 `-1`，并设置 `errno`。

## 知识点补充

- `p` 表示 presentation，也就是人可读字符串形式。
- `n` 表示 network，也就是网络字节序二进制形式。
- `inet_pton` 常用于客户端填写服务器 IP。

## 常见用法

```c
struct sockaddr_in servaddr;
inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr.s_addr);
```

## 易错点

- `inet_pton` 不能解析 `"www.baidu.com"` 这种域名。
- 返回 `0` 表示地址字符串格式不合法，不是系统调用失败。
- 第三个参数要传目标地址字段的地址。

## 相关概念

- [[linux网络编程/概念词条/IP地址|IP地址]]
- [[linux网络编程/概念词条/地址族|地址族]]
- [[linux网络编程/概念词条/in_addr|in_addr]]
- [[linux网络编程/概念词条/sockaddr_in|sockaddr_in]]

## 相关课时

- [[linux网络编程/课时笔记/02 Socket编程基础/03 IP地址转换函数|03 IP地址转换函数]]

## 相关模块

- [[linux网络编程/02 Socket编程基础|02 Socket编程基础]]
