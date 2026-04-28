---
title: Workflow-08 路由、DNS、协议层
tags:
  - workflow
  - dns
  - protocol
  - interview
---

# Workflow-08 路由、DNS、协议层

相关笔记：
[[Workflow项目面试拆解]] | [[Workflow-06 kernel 层：Communicator 与 CommScheduler]] | [[Workflow-07 上层模块：WFTaskFactory、Server、Client]] | [[Workflow-09 高频面试题与背诵稿]]

## 这一章的定位

这一章专门讲容易被追问、但又不是最先看的部分：

- 请求发到哪里
- 域名怎么解析
- 路由结果怎么管理
- 协议层怎么把字节流变成对象

## `WFNameService`

文件：`src/nameservice/WFNameService.h`

它是名字服务策略注册中心。

### 它解决的问题

- 不同 scheme 或名字可以绑定不同解析策略
- 需要一个统一入口去创建路由任务

### 核心函数

| 函数 | 输入 | 输出 | 作用 |
|---|---|---|---|
| `add_policy(const char *name, WFNSPolicy *policy)` | 名称、策略 | `int` | 注册策略 |
| `get_policy(const char *name)` | 名称 | `WFNSPolicy *` | 获取策略 |
| `del_policy(const char *name)` | 名称 | `WFNSPolicy *` | 删除策略 |
| `get_default_policy()` | 无 | `WFNSPolicy *` | 获取默认策略 |
| `set_default_policy(WFNSPolicy *policy)` | 策略 | 无 | 设置默认策略 |

## `WFDnsResolver`

文件：`src/nameservice/WFDnsResolver.h`

这是默认 DNS 解析策略。

### 它的作用

- 看 URI 中的 host
- 判断是否需要 DNS
- 发起本地线程 DNS 或网络 DNS
- 生成路由结果

### `WFResolverTask` 关键函数

| 函数 | 输入 | 输出 | 作用 |
|---|---|---|---|
| `dispatch()` | 无 | 无 | 启动解析流程 |
| `done()` | 无 | `SubTask *` | 结束后返回下一个任务 |
| `request_dns()` | 无 | 无 | 发起 DNS 查询 |
| `task_callback()` | 无 | 无 | 收尾并进入上层回调 |

## `WFGlobal`

文件：`src/manager/WFGlobal.h`

它是全局配置和全局资源入口。

### 它管理的东西

- 全局 scheduler
- 全局 SSL
- DNS 相关对象
- 线程池执行器
- 路由管理器
- DNS 缓存

### 常用函数

| 函数 | 输入 | 输出 | 作用 |
|---|---|---|---|
| `get_scheduler()` | 无 | `CommScheduler *` | 获取全局调度器 |
| `get_compute_executor()` | 无 | `Executor *` | 获取计算执行器 |
| `get_dns_client()` | 无 | `WFDnsClient *` | 获取 DNS 客户端 |
| `get_dns_resolver()` | 无 | `WFDnsResolver *` | 获取 DNS 解析器 |
| `get_name_service()` | 无 | `WFNameService *` | 获取名字服务 |
| `get_error_string(int state, int error)` | 状态、错误码 | `const char *` | 获取错误文本 |

## `UpstreamManager`

文件：`src/manager/UpstreamManager.h`

可以理解成本地版 Nginx upstream 管理器。

### 它解决的问题

- 一个逻辑服务名背后有多台机器
- 需要负载均衡
- 需要主备或分组
- 需要禁用或恢复节点

### 常见能力

- round robin
- weighted random
- consistent hash
- manual select

## 协议层

文件主要在 `src/protocol/`

### `HttpMessage`

文件：`src/protocol/HttpMessage.h`

它负责：

- 保存 HTTP 结构化数据
- 编码 HTTP 消息
- 解析 HTTP 消息

### 核心函数

| 函数 | 输入 | 输出 | 作用 |
|---|---|---|---|
| `get_http_version()` | 无 | 版本字符串 | 获取版本 |
| `set_http_version(version)` | 版本字符串 | `bool` | 设置版本 |
| `add_header_pair(name, value)` | 键值 | `bool` | 添加 header |
| `set_header_pair(name, value)` | 键值 | `bool` | 覆盖 header |
| `get_parsed_body(...)` | 输出指针 | `bool` | 获取已解析 body |
| `append_output_body(...)` | body 内容 | `bool` | 追加发送 body |
| `is_chunked()` | 无 | `bool` | 是否 chunked |
| `is_keep_alive()` | 无 | `bool` | 是否 keep alive |

### 其他协议怎么概括

- Redis：RESP 编解码
- MySQL：报文和结果集解析
- DNS：请求构造和响应解析
- Kafka：消息和结果对象封装

## 面试里的总结

这一层是把“请求到底发给谁”以及“网络字节到底怎么解析”这两个问题补完整。`WFNameService/WFDnsResolver/UpstreamManager` 负责地址和路由决策，`protocol` 负责协议编解码。
