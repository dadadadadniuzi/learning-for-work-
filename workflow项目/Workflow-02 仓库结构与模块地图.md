---
title: Workflow-02 仓库结构与模块地图
tags:
  - workflow
  - architecture
  - interview
---

# Workflow-02 仓库结构与模块地图

相关笔记：
[[Workflow项目面试拆解]] | [[Workflow-01 项目定位与一句话介绍]] | [[Workflow-05 核心抽象：SubTask、SeriesWork、ParallelWork、Workflow]] | [[Workflow-06 kernel 层：Communicator 与 CommScheduler]]

## 仓库结构

```text
workflow-master/
├─ src/
│  ├─ kernel/
│  ├─ factory/
│  ├─ protocol/
│  ├─ server/
│  ├─ client/
│  ├─ manager/
│  ├─ nameservice/
│  ├─ util/
│  └─ include/
├─ tutorial/
├─ docs/
├─ test/
└─ benchmark/
```

## 每个目录怎么讲

### `src/kernel`

最底层的运行时内核。

负责：

- 通信会话
- 调度器
- poller
- 线程池
- 睡眠定时器
- 文件 IO 服务

一句话：这是“真正干活”的底层引擎。

### `src/factory`

把底层能力包装成面向用户的任务。

负责：

- `WFTaskFactory`
- `WFTask`
- `Workflow`
- `WFGraphTask`

一句话：这是“用户创建任务和编排任务”的入口层。

### `src/protocol`

负责协议对象和协议解析。

包括：

- HTTP
- Redis
- MySQL
- DNS
- Kafka
- TLV

一句话：这是“字节流和结构化消息之间的转换层”。

### `src/server`

服务端封装层。

包括：

- `WFServer`
- `WFHttpServer`
- `WFRedisServer`
- `WFMySQLServer`
- `WFDnsServer`

一句话：这是“监听、收请求、回响应”的统一封装。

### `src/client`

客户端封装层。

包括：

- `WFDnsClient`
- `WFHttpChunkedClient`
- Redis/MySQL/Kafka 等客户端能力

一句话：这是“用户侧发请求”的高层接口。

### `src/manager`

全局资源和管理能力。

包括：

- `WFGlobal`
- `RouteManager`
- `DnsCache`
- `UpstreamManager`

一句话：这是“全局配置、全局资源、全局路由”的管理层。

### `src/nameservice`

名字服务和路由策略层。

包括：

- `WFNameService`
- `WFDnsResolver`
- `WFServiceGovernance`

一句话：这是“请求应该发到哪里”的决策层。

### `src/util`

工具类和基础函数。

包括：

- URI 解析
- 编码流
- 字符串工具
- LRU 缓存

### `src/include`

对外暴露的公共头文件目录。

一句话：这是最终用户 include 的接口集合。

## 仓库外围目录怎么讲

### `tutorial`

新手示例，最适合面试前快速理解框架怎么用。

### `docs`

文档和教程说明。

### `test`

单元测试，说明作者对模块级能力做了验证。

### `benchmark`

性能测试，说明项目强调高性能和吞吐。

## 面试中的讲法

如果面试官让你“介绍一下仓库结构”，你不要按目录念文件名，最好这样说：

这个项目从下往上大致分成四层：最底下是 `kernel` 的异步运行时，中间是 `factory` 的任务包装和工作流编排，上面是 `protocol` 的协议实现，再上层是 `server/client/nameservice/manager` 这些真正给业务使用的模块。外围还有 `tutorial/docs/test/benchmark`，分别对应示例、文档、测试和性能验证。
