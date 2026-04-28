---
title: Workflow 项目面试拆解
tags:
  - workflow
  - interview
  - index
aliases:
  - workflow-master 面试笔记
---

# Workflow 项目面试拆解

> [!info]
> 这是一页索引笔记，对应仓库 `G:\计算机学习\workflow项目\workflow-master`。
> 现在内容已经拆成多份章节笔记，适合在 Obsidian 里逐块复习。

## 快速入口

- [[Workflow-01 项目定位与一句话介绍]]
- [[Workflow-02 仓库结构与模块地图]]
- [[Workflow-03 从零实现这个框架的思路]]
- [[Workflow-04 HTTP 请求与服务端链路]]
- [[Workflow-05 核心抽象：SubTask、SeriesWork、ParallelWork、Workflow]]
- [[Workflow-06 kernel 层：Communicator 与 CommScheduler]]
- [[Workflow-07 上层模块：WFTaskFactory、Server、Client]]
- [[Workflow-08 路由、DNS、协议层]]
- [[Workflow-09 高频面试题与背诵稿]]

## 建议复习顺序

> [!tip]
> 如果时间紧，按下面顺序看最划算。

1. [[Workflow-01 项目定位与一句话介绍]]
2. [[Workflow-03 从零实现这个框架的思路]]
3. [[Workflow-04 HTTP 请求与服务端链路]]
4. [[Workflow-05 核心抽象：SubTask、SeriesWork、ParallelWork、Workflow]]
5. [[Workflow-09 高频面试题与背诵稿]]

## 按问题找笔记

- 想回答“这个项目到底是什么”：
  看 [[Workflow-01 项目定位与一句话介绍]]
- 想回答“目录和模块怎么讲”：
  看 [[Workflow-02 仓库结构与模块地图]]
- 想回答“如果你自己从零实现，会怎么做”：
  看 [[Workflow-03 从零实现这个框架的思路]]
- 想回答“一个 HTTP 请求在里面怎么流转”：
  看 [[Workflow-04 HTTP 请求与服务端链路]]
- 想回答“核心数据结构和任务模型是什么”：
  看 [[Workflow-05 核心抽象：SubTask、SeriesWork、ParallelWork、Workflow]]
- 想回答“底层为什么能异步高性能”：
  看 [[Workflow-06 kernel 层：Communicator 与 CommScheduler]]
- 想回答“用户平时真正用哪些 API”：
  看 [[Workflow-07 上层模块：WFTaskFactory、Server、Client]]
- 想回答“DNS、路由、upstream、协议层怎么讲”：
  看 [[Workflow-08 路由、DNS、协议层]]

## 你的面试主线

可以先背这句话：

这个项目不是业务系统，而是一个 C++ 异步网络工作流框架。它把网络请求、文件 IO、定时器、计算任务统一抽象成 Task，再通过 `SeriesWork` 和 `ParallelWork` 编排成工作流，底层由 `Communicator` 和 `CommScheduler` 负责非阻塞通信和调度，上层通过 `WFTaskFactory`、Server、Client、DNS、路由和协议模块把这些能力封装成可直接使用的接口。
