---
title: 静态Web服务器基础
tags: [linux, 网络编程, 课时笔记, 网络编程/Web]
---
# 静态Web服务器基础

## 本节学什么

- [[linux网络编程/概念词条/Web服务器|Web 服务器]] 的最小实现思路
- 请求解析、资源定位、响应拼接
- [[linux网络编程/概念词条/静态资源|静态资源]] 返回逻辑
- [[linux网络编程/概念词条/HTTP状态码|HTTP 状态码]] 和 [[linux网络编程/概念词条/MIME类型|MIME 类型]] 在代码里的作用

## 本节学什么详解

- 最小实现思路：服务器先接收 TCP 连接，再读取请求报文，解析请求路径，根据路径找到文件，最后拼接 HTTP 响应返回。
- 请求解析：至少要从请求行里拿到请求方法和资源路径。
- 资源定位：把 URL 路径映射到服务器目录中的真实文件路径。
- 响应拼接：成功时返回 `200 OK` 和文件内容；失败时返回 `404 Not Found` 或 `500 Internal Server Error`。
- [[linux网络编程/概念词条/MIME类型|MIME 类型]]：决定响应头里 `Content-Type` 应该写什么。

## 知识点补充

- 简化版 Web 服务器通常只处理 `GET`。
- 课程项目里，最容易出错的是路径解析、文件不存在判断、响应头拼接和内容类型判断。
- 如果要支持多个客户端，就会回到前面学过的多线程、epoll、Reactor 或 Libevent。

## 本节内容速览

- `accept` 客户端连接。
- `read/recv` 读取请求。
- 解析请求路径。
- 读文件或判断 404。
- 拼接响应头和响应体。
- `send/write` 返回数据。

## 复习时要回答

- 静态 Web 服务器和普通 TCP 回显服务器相比，多了哪几步？
- 为什么返回 HTML 时要写 `Content-Type`？
- 一个最小的 404 响应应该包含什么？

## 本节关键函数

- [[linux网络编程/函数笔记/Socket/accept|accept]]
- [[linux网络编程/函数笔记/Socket/read|read]]
- [[linux网络编程/函数笔记/Socket/recv|recv]]
- [[linux网络编程/函数笔记/Socket/send|send]]
- [[linux系统编程/函数笔记/文件IO/open|open]]

## 本节关键概念

- [[linux网络编程/概念词条/Web服务器|Web 服务器]]
- [[linux网络编程/概念词条/静态资源|静态资源]]
- [[linux网络编程/概念词条/HTTP请求报文|HTTP 请求报文]]
- [[linux网络编程/概念词条/HTTP响应报文|HTTP 响应报文]]
- [[linux网络编程/概念词条/HTTP状态码|HTTP 状态码]]
- [[linux网络编程/概念词条/MIME类型|MIME 类型]]

## 关联模块

- [[linux网络编程/08 Web大作业|08 Web大作业]]
