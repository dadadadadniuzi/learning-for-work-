---
title: sockaddr与sockaddr_in结构
tags:
  - linux
  - 网络编程
  - 课时笔记
  - 网络编程/Socket编程基础
---
# sockaddr与sockaddr_in结构

## 本节学什么

- [[linux网络编程/概念词条/sockaddr|sockaddr]]
- [[linux网络编程/概念词条/sockaddr_in|sockaddr_in]]
- [[linux网络编程/概念词条/in_addr|in_addr]]
- [[linux网络编程/概念词条/socklen_t|socklen_t]]
- `(struct sockaddr *)&addr` 为什么常见

## 本节学什么详解

- [[linux网络编程/概念词条/sockaddr|sockaddr]]：通用地址结构，socket API 为了兼容不同协议族，参数常写成 `struct sockaddr *`。
- [[linux网络编程/概念词条/sockaddr_in|sockaddr_in]]：IPv4 专用地址结构，实际保存 IPv4 的地址族、端口和 IP 地址。
- [[linux网络编程/概念词条/in_addr|in_addr]]：`sockaddr_in` 里的 IPv4 地址字段类型，核心成员通常是 `s_addr`。
- [[linux网络编程/概念词条/socklen_t|socklen_t]]：表示 socket 地址结构长度的类型，常和 `sizeof(addr)`、`accept` 的传入传出参数一起出现。
- `(struct sockaddr *)&addr`：实际准备的是 `sockaddr_in`，但函数形参需要通用 `sockaddr *`，所以调用时做指针类型转换。

## 知识点补充

- `sin_family` 通常填 `AF_INET`。
- `sin_port` 必须是网络字节序，常用 `htons(port)`。
- `sin_addr.s_addr` 必须是网络字节序 IPv4 地址，可用 `htonl(INADDR_ANY)` 或 `inet_pton`。

## 本节内容速览

- `sockaddr` 是通用壳。
- `sockaddr_in` 是 IPv4 地址结构。
- `socklen_t` 表示地址结构大小。

## 复习时要回答

- `sockaddr` 和 `sockaddr_in` 是什么关系？
- `sin_port` 为什么要用 `htons`？
- `accept` 里的 `addrlen` 为什么是指针？

## 本节关键概念

- [[linux网络编程/概念词条/sockaddr|sockaddr]]
- [[linux网络编程/概念词条/sockaddr_in|sockaddr_in]]
- [[linux网络编程/概念词条/in_addr|in_addr]]
- [[linux网络编程/概念词条/socklen_t|socklen_t]]
- [[linux网络编程/概念词条/INADDR_ANY|INADDR_ANY]]

## 关联模块

- [[linux网络编程/02 Socket编程基础|02 Socket编程基础]]
