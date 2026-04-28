---
title: TinyWebServer 面试拆解笔记
aliases:
  - TinyWebServer Interview Notes
  - TinyWebServer 项目详解
tags:
  - project
  - interview
  - network
  - cpp
  - webserver
created: 2026-04-24
---

# TinyWebServer 面试拆解笔记

> [!abstract]
> 这页现在作为总览索引使用，不再把所有内容堆在一篇里。下面每一章都是独立笔记，可以直接在 Obsidian 里跳转复习。

## 使用方式

- 如果你只剩 5 分钟：先看 [[TinyWebServer-10分钟速背版]]
- 如果你想练回答：看 [[TinyWebServer-面试问答模拟版]]
- 如果你要系统理解项目：按下面章节顺序看

## 章节导航

1. [[TinyWebServer-拆解/01-项目总览与架构]]
2. [[TinyWebServer-拆解/02-请求链路与从零实现]]
3. [[TinyWebServer-拆解/03-入口与WebServer总控]]
4. [[TinyWebServer-拆解/04-http_conn与HTTP状态机]]
5. [[TinyWebServer-拆解/05-线程池与并发模型]]
6. [[TinyWebServer-拆解/06-定时器与连接管理]]
7. [[TinyWebServer-拆解/07-数据库连接池与登录注册]]
8. [[TinyWebServer-拆解/08-日志系统与同步原语]]
9. [[TinyWebServer-拆解/09-静态资源、项目亮点与面试表达]]
10. [[TinyWebServer-拆解/10-HTTP请求报文解析与GETPOST流程]]
11. [[TinyWebServer-拆解/11-Webbench压力测试与性能瓶颈分析]]
12. [[TinyWebServer-拆解/12-半同步半反应堆与并发模型详解]]
13. [[TinyWebServer-拆解/13-按代码读TinyWebServer的顺序]]

## 推荐阅读顺序

### 第一轮：先建立整体感

1. [[TinyWebServer-拆解/01-项目总览与架构]]
2. [[TinyWebServer-拆解/02-请求链路与从零实现]]
3. [[TinyWebServer-拆解/03-入口与WebServer总控]]
4. [[TinyWebServer-拆解/04-http_conn与HTTP状态机]]

### 第二轮：补并发和资源管理

1. [[TinyWebServer-拆解/05-线程池与并发模型]]
2. [[TinyWebServer-拆解/06-定时器与连接管理]]
3. [[TinyWebServer-拆解/07-数据库连接池与登录注册]]
4. [[TinyWebServer-拆解/08-日志系统与同步原语]]

### 第三轮：面试表达

1. [[TinyWebServer-拆解/09-静态资源、项目亮点与面试表达]]
2. [[TinyWebServer-面试问答模拟版]]

## 你明天最少要记住的 10 句话

1. 这是一个基于 `epoll + 线程池 + 非阻塞 I/O` 的轻量级 Web 服务器。
2. 主线程负责事件监听和分发，工作线程负责请求处理。
3. `http_conn` 表示一个客户端连接，内部维护 HTTP 解析状态。
4. 项目用状态机解析 HTTP，因为 TCP 是流式传输。
5. 项目支持 Reactor 和 Proactor 两种并发模型。
6. MySQL 连接池通过复用连接减少数据库访问开销。
7. 定时器负责关闭长时间不活跃的连接。
8. 日志系统支持同步和异步两种模式，异步通过阻塞队列实现。
9. 静态文件使用 `mmap + writev` 返回，提高传输效率。
10. 项目更偏重网络和并发模型展示，而不是复杂业务功能。

## 关联笔记

- [[TinyWebServer-10分钟速背版]]
- [[TinyWebServer-10分钟总复盘版]]
- [[TinyWebServer-面试问答模拟版]]
