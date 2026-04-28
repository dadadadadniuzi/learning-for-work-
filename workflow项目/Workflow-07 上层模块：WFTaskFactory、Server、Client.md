---
title: Workflow-07 上层模块：WFTaskFactory、Server、Client
tags:
  - workflow
  - api
  - interview
---

# Workflow-07 上层模块：WFTaskFactory、Server、Client

相关笔记：
[[Workflow项目面试拆解]] | [[Workflow-04 HTTP 请求与服务端链路]] | [[Workflow-05 核心抽象：SubTask、SeriesWork、ParallelWork、Workflow]] | [[Workflow-08 路由、DNS、协议层]]

## 这一章在回答什么

这一章回答的是：

- 平时业务开发真正会直接用哪些类？
- 用户是怎么通过框架创建任务、启动服务、发请求的？

## `WFTask`

文件：`src/factory/WFTask.h`

它定义了各种任务对象的统一包装。

### `WFNetworkTask<REQ, RESP>`

这是 HTTP/Redis/MySQL/DNS 任务的共同基类。

| 函数 | 输入 | 输出 | 作用 |
|---|---|---|---|
| `start()` | 无 | 无 | 启动任务 |
| `dismiss()` | 无 | 无 | 放弃任务 |
| `get_req()` | 无 | `REQ *` | 获取请求对象 |
| `get_resp()` | 无 | `RESP *` | 获取响应对象 |
| `get_state()` | 无 | `int` | 获取状态 |
| `get_error()` | 无 | `int` | 获取错误码 |
| `set_send_timeout(int timeout)` | 毫秒 | 无 | 设置发送超时 |
| `set_receive_timeout(int timeout)` | 毫秒 | 无 | 设置接收超时 |
| `set_keep_alive(int timeout)` | 毫秒 | 无 | 设置保活 |
| `set_watch_timeout(int timeout)` | 毫秒 | 无 | 设置首包等待超时 |
| `set_prepare(...)` | prepare 回调 | 无 | 设置发送前准备逻辑 |
| `set_callback(...)` | callback 回调 | 无 | 设置结束回调 |

### `WFTimerTask`

| 函数 | 输入 | 输出 | 作用 |
|---|---|---|---|
| `start()` | 无 | 无 | 启动定时器 |
| `dismiss()` | 无 | 无 | 放弃定时器 |
| `get_state()` | 无 | `int` | 获取状态 |
| `get_error()` | 无 | `int` | 获取错误 |

## `WFTaskFactory`

文件：`src/factory/WFTaskFactory.h`

这是业务层最重要的入口。

### 常用工厂函数

| 函数 | 输入 | 输出 | 作用 |
|---|---|---|---|
| `create_http_task(url, redirect_max, retry_max, callback)` | URL、重定向次数、重试次数、回调 | `WFHttpTask *` | 创建 HTTP 客户端任务 |
| `create_redis_task(url, retry_max, callback)` | URL、重试次数、回调 | `WFRedisTask *` | 创建 Redis 任务 |
| `create_mysql_task(url, retry_max, callback)` | URL、重试次数、回调 | `WFMySQLTask *` | 创建 MySQL 任务 |
| `create_dns_task(url, retry_max, callback)` | URL、重试次数、回调 | `WFDnsTask *` | 创建 DNS 任务 |
| `create_timer_task(seconds, nanoseconds, callback)` | 秒、纳秒、回调 | `WFTimerTask *` | 创建定时器任务 |
| `create_pread_task(...)` | 文件参数 | `WFFileIOTask *` | 创建文件读任务 |
| `create_pwrite_task(...)` | 文件参数 | `WFFileIOTask *` | 创建文件写任务 |

### 面试里的讲法

`WFTaskFactory` 的作用就是把底层复杂对象统一包装成简单任务，用户拿到后只需要填请求、设置回调、再启动就可以了。

## Server 层

文件：`src/server/WFServer.h`

### `WFServerBase`

负责所有 server 的通用能力：

- `start()`
- `serve()`
- `stop()`
- `shutdown()`
- `wait_finish()`

### `WFServer<REQ, RESP>`

模板服务端，负责把协议和用户回调绑在一起。

最重要的点：

- 新连接到来时会创建任务
- 任务里持有请求和响应对象
- 用户只需要写 `process(task)`

### 面试里的讲法

服务端层本质上是把监听、accept、协议解析、回包、keep-alive 这些流程统一封装起来，业务代码只需要操作请求对象和响应对象。

## Client 层

常见类包括：

- `WFDnsClient`
- `WFHttpChunkedClient`
- MySQL/Redis/Kafka 相关客户端封装

### `WFDnsClient`

| 函数 | 输入 | 输出 | 作用 |
|---|---|---|---|
| `init(...)` | DNS 服务 URL 和参数 | `int` | 初始化客户端 |
| `deinit()` | 无 | 无 | 销毁客户端 |
| `create_dns_task(name, callback)` | 域名、回调 | `WFDnsTask *` | 创建 DNS 查询任务 |

### `WFHttpChunkedClient`

适合处理流式 HTTP chunk 场景。

关键点：

- 能按 chunk 提取数据
- 能分别控制首包超时和整体接收超时
- 最终还是基于底层 HTTP 任务

## 面试总结

上层模块的本质是把底层复杂的异步引擎变成“能直接拿来用的 API”。其中 `WFTaskFactory` 是任务入口，Server 是收请求入口，Client 是发请求入口。
