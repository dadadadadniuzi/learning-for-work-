---
title: Workflow-01 项目定位与一句话介绍
tags:
  - workflow
  - interview
  - overview
---

# Workflow-01 项目定位与一句话介绍

相关笔记：
[[Workflow项目面试拆解]] | [[Workflow-02 仓库结构与模块地图]] | [[Workflow-09 高频面试题与背诵稿]]

## 一句话介绍

`workflow-master` 不是一个业务网站项目，而是一个 **C++ 异步网络工作流框架**。

你可以把它理解成：

**它把网络 IO、文件 IO、定时器、计算任务统一抽象成 Task，再把这些 Task 编排成 Workflow。**

## 它到底解决了什么问题

传统 C++ 网络开发里，开发者经常要自己处理：

- socket 建连和收发
- epoll 或 poller
- 线程池
- 超时
- 回调嵌套
- 协议解析
- 多任务串并联逻辑

这个框架的目标就是把这些通用复杂度收进框架里，让业务层只关心：

- 我要创建什么任务
- 任务之间是什么关系
- 任务完成后我要怎么处理结果

## 这个项目实现了哪些能力

- 异步 HTTP/Redis/MySQL/DNS/Kafka 客户端
- 异步 HTTP/MySQL/Redis/DNS 服务端
- 定时器任务
- 文件异步 IO 任务
- 线程池计算任务
- 串行和并行工作流
- 名字服务、DNS 解析、路由、upstream、服务治理

## 30 秒版本

这个项目本质上是一个 C++ 异步网络框架。它把 HTTP、Redis、MySQL、DNS 这些网络请求，连同定时器、文件 IO、计算任务统一抽象成任务对象，开发者通过 `WFTaskFactory` 创建任务，再通过 `Workflow`、`SeriesWork`、`ParallelWork` 组织任务关系。底层由 `Communicator` 和 `CommScheduler` 负责非阻塞 IO、连接管理和事件调度。

## 2 分钟版本

从架构上看，这个项目可以分成几层：

1. `kernel`：底层通信、poller、线程池、调度器。
2. `factory`：把底层能力包装成用户能直接创建的任务。
3. `Workflow / SeriesWork / ParallelWork`：把任务组织成串行流、并行流。
4. `protocol`：HTTP、Redis、MySQL、DNS、Kafka 等协议编解码。
5. `server / client / nameservice / manager`：服务端、客户端、名字服务、路由、upstream、DNS 和全局配置。

## 面试里最重要的定位句

如果面试官问“这个项目是什么”，最稳的说法是：

它是一个偏基础设施的 C++ 异步网络框架，不是业务系统。它的重点不是某个具体接口，而是统一任务模型、异步调度模型和工作流编排能力。
