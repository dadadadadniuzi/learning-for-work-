---
title: Workflow-10 10分钟速背版
tags:
  - workflow
  - interview
  - quick-review
aliases:
  - Workflow 十分钟速背
---

# Workflow-10 10分钟速背版

相关笔记：
[[Workflow项目面试拆解]] | [[Workflow-01 项目定位与一句话介绍]] | [[Workflow-04 HTTP 请求与服务端链路]] | [[Workflow-09 高频面试题与背诵稿]]

> [!important]
> 这份笔记只保留面试前 10 分钟最该背的内容。
> 如果你时间非常紧，只看这一页也能把项目主线讲出来。

## 1. 一句话先背下来

这个项目不是业务系统，而是一个 **C++ 异步网络工作流框架**。它把网络请求、文件 IO、定时器、计算任务统一抽象成 Task，再通过 `SeriesWork` 和 `ParallelWork` 编排成 Workflow，底层由 `Communicator` 和 `CommScheduler` 负责非阻塞通信和调度。

## 2. 它到底是干什么的

它主要解决的是 C++ 网络开发里这些麻烦事：

- socket 和异步 IO
- 线程池和调度
- 协议解析
- 超时控制
- 回调嵌套
- 多任务串并联

框架把这些复杂度封装起来，业务层只需要：

- 创建任务
- 组织任务关系
- 在 callback 里处理结果

## 3. 目录结构只背这 4 层

1. `kernel`
   底层异步运行时，负责通信、poller、线程池、调度。
2. `factory`
   任务工厂和工作流编排层。
3. `protocol`
   HTTP、Redis、MySQL、DNS 等协议编解码。
4. `server/client/nameservice/manager`
   面向业务使用的高层模块。

## 4. 核心抽象只背这 4 个

### `SubTask`

最小任务单元。

- `dispatch()`：启动任务
- `done()`：结束后返回下一个任务

### `SeriesWork`

串行工作流。

- 表达“先做 A，再做 B”

### `ParallelWork`

并行工作流。

- 表达“同时做多个子流程，全部完成后再汇总”

### `Workflow`

工作流创建入口。

- 创建和启动 `SeriesWork`
- 创建和启动 `ParallelWork`

## 5. 用户最常用的入口

### `WFTaskFactory`

最常见的任务创建入口：

- `create_http_task()`
- `create_redis_task()`
- `create_mysql_task()`
- `create_dns_task()`
- `create_timer_task()`

一句话：它把底层复杂能力包装成业务能直接用的任务对象。

### `WFHttpServer`

最常见的服务端入口。

用户只需要写一个处理函数：

- 收到 `WFHttpTask`
- 通过 `get_req()` 看请求
- 通过 `get_resp()` 写响应

## 6. HTTP 客户端链路怎么背

背这条线：

`WFTaskFactory::create_http_task()`
-> `WFHttpTask`
-> `Workflow / SeriesWork`
-> `CommScheduler::request()`
-> `Communicator`
-> `HttpMessage` 编解码
-> callback

口语化说法：

用户先创建一个 HTTP 任务，任务启动后由调度器选择目标和连接，通信引擎负责 connect、send、recv，协议层把字节流解析成响应对象，最后触发回调。

## 7. HTTP 服务端链路怎么背

背这条线：

`WFHttpServer::start()`
-> bind/listen
-> 新连接
-> 创建 task/session
-> 解析请求
-> 用户 `process(task)`
-> 填充响应
-> 回写响应

口语化说法：

服务端启动监听后，收到连接会创建任务对象，框架先解析 HTTP 请求，再执行用户回调，用户在回调里填充响应对象，最后框架负责发回客户端。

## 8. 底层核心只背这两个类

### `Communicator`

底层通信引擎，负责：

- connect
- send
- recv
- reply
- 定时器
- 文件 IO 服务接入

### `CommScheduler`

调度外壳，负责：

- 选择目标
- 分配连接
- 控制负载

一句话：

`Communicator` 负责“怎么异步通信”，`CommScheduler` 负责“请求交给谁”。

## 9. 进阶模块只背关键词

- `WFNameService`
  名字服务策略中心
- `WFDnsResolver`
  DNS 解析策略
- `WFGlobal`
  全局配置和全局资源入口
- `UpstreamManager`
  本地版 upstream 和负载均衡管理
- `HttpMessage`
  HTTP 协议对象，负责编解码

## 10. 高频面试题标准回答

### 这个项目是什么

它是一个 C++ 异步网络工作流框架，不是业务系统。

### 它和普通线程池项目有什么区别

它不只是调度计算任务，还统一处理网络 IO、服务端、协议解析、DNS、路由和工作流编排。

### 它为什么快

因为它是异步非阻塞模型，不会一请求一线程，而且支持统一调度、连接复用和 keep-alive。

### 最核心的设计是什么

先把不同能力统一抽象成 Task，再统一编排成 Workflow。

## 11. 最后一分钟只看这段

这个项目是一个 C++ 异步网络工作流框架。它把网络请求、文件 IO、定时器和计算任务统一抽象成任务，通过 `SeriesWork` 和 `ParallelWork` 组织成工作流。底层由 `Communicator` 和 `CommScheduler` 提供异步通信和调度，上层通过 `WFTaskFactory`、Server、Client、协议层、DNS 和路由模块把这些能力封装成开发者可直接使用的接口。
