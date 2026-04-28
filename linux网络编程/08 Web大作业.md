---
title: Web大作业
tags: [linux, 网络编程, 模块复习, 网络编程/Web]
---
# Web大作业

## 本章目标

- 理解 [[linux网络编程/概念词条/HTTP|HTTP]] 在应用层是如何组织浏览器和服务器通信的。
- 掌握 [[linux网络编程/概念词条/HTTP请求报文|HTTP 请求报文]]、[[linux网络编程/概念词条/HTTP响应报文|HTTP 响应报文]]、[[linux网络编程/概念词条/HTTP状态码|HTTP 状态码]] 的基本结构。
- 理解浏览器访问网页时“发请求、收响应、解析资源”的完整流程。
- 把前面学过的 [[linux网络编程/概念词条/TCP|TCP]]、[[linux网络编程/概念词条/IO多路复用|IO 多路复用]]、[[linux网络编程/概念词条/Reactor反应堆模式|Reactor]] 等知识串到一个简单 Web 服务器项目里。
- 为课程综合实验、静态 Web 服务器和后续更完整的后端开发知识做过渡。

## 核心函数

- [[linux网络编程/函数笔记/Socket/socket|socket]]
- [[linux网络编程/函数笔记/Socket/bind|bind]]
- [[linux网络编程/函数笔记/Socket/listen|listen]]
- [[linux网络编程/函数笔记/Socket/accept|accept]]
- [[linux网络编程/函数笔记/Socket/read|read]]
- [[linux网络编程/函数笔记/Socket/recv|recv]]
- [[linux网络编程/函数笔记/Socket/send|send]]
- [[linux网络编程/函数笔记/Socket/shutdown|shutdown]]

## 本模块概念

- [[linux网络编程/概念词条/HTTP|HTTP]]
- [[linux网络编程/概念词条/URL|URL]]
- [[linux网络编程/概念词条/HTTP方法|HTTP 方法]]
- [[linux网络编程/概念词条/HTTP请求报文|HTTP 请求报文]]
- [[linux网络编程/概念词条/HTTP响应报文|HTTP 响应报文]]
- [[linux网络编程/概念词条/HTTP状态码|HTTP 状态码]]
- [[linux网络编程/概念词条/MIME类型|MIME 类型]]
- [[linux网络编程/概念词条/Web服务器|Web 服务器]]
- [[linux网络编程/概念词条/静态资源|静态资源]]
- [[linux网络编程/概念词条/浏览器访问服务器流程|浏览器访问服务器流程]]

## 本模块课时

- [[linux网络编程/课时笔记/08 Web大作业/01 HTTP与Web基础|01 HTTP与Web基础]]
- [[linux网络编程/课时笔记/08 Web大作业/02 HTTP请求与响应报文|02 HTTP请求与响应报文]]
- [[linux网络编程/课时笔记/08 Web大作业/03 浏览器访问服务器流程|03 浏览器访问服务器流程]]
- [[linux网络编程/课时笔记/08 Web大作业/04 静态Web服务器基础|04 静态Web服务器基础]]

## 本模块指令

- [[linux网络编程/指令查询/命令卡/curl|curl]]
- [[linux网络编程/指令查询/命令卡/telnet|telnet]]
- [[linux网络编程/指令查询/命令卡/nc|nc]]

## 细节补充

- HTTP 是应用层协议，底层仍然常常跑在 [[linux网络编程/概念词条/TCP|TCP]] 之上。
- 浏览器访问网页时，并不是“只发一次请求就结束”，而是可能先请求 HTML，再继续请求 CSS、JS、图片等 [[linux网络编程/概念词条/静态资源|静态资源]]。
- 简单 Web 服务器的本质，仍然是“接收 TCP 连接 -> 读取请求文本 -> 解析请求行/请求头 -> 拼接响应报文 -> 把资源发回去”。
- 课程里的 Web 大作业通常是把前面章节的 socket、并发、IO 多路复用、Reactor 思路汇总到一个更接近真实应用的场景中。

## 复习路线

- 先理解 [[linux网络编程/概念词条/HTTP|HTTP]] 和 [[linux网络编程/概念词条/URL|URL]] 的作用。
- 再把 [[linux网络编程/概念词条/HTTP请求报文|请求报文]]、[[linux网络编程/概念词条/HTTP响应报文|响应报文]] 的结构背清楚。
- 然后理解 [[linux网络编程/概念词条/浏览器访问服务器流程|浏览器访问服务器流程]]。
- 最后把这些知识落实到 [[linux网络编程/概念词条/Web服务器|Web 服务器]] 返回 [[linux网络编程/概念词条/静态资源|静态资源]] 的代码流程上。
