---
title: Workflow-05 核心抽象：SubTask、SeriesWork、ParallelWork、Workflow
tags:
  - workflow
  - core
  - interview
---

# Workflow-05 核心抽象：SubTask、SeriesWork、ParallelWork、Workflow

相关笔记：
[[Workflow项目面试拆解]] | [[Workflow-03 从零实现这个框架的思路]] | [[Workflow-04 HTTP 请求与服务端链路]] | [[Workflow-06 kernel 层：Communicator 与 CommScheduler]]

## 这一章的定位

这一章回答的是：

- 这个框架里“任务”到底是什么？
- 任务之间怎么串起来？

## `SubTask`

文件：`src/kernel/SubTask.h`

`SubTask` 是最小任务抽象。

### 关键函数

| 函数 | 输入 | 输出 | 作用 |
|---|---|---|---|
| `dispatch()` | 无 | 无 | 启动任务 |
| `done()` | 无 | `SubTask *` | 当前任务结束后返回下一个任务 |
| `subtask_done()` | 无 | 无 | 统一完成入口 |
| `get_pointer()` | 无 | `void *` | 获取挂载上下文 |
| `set_pointer(void *pointer)` | 上下文指针 | 无 | 设置挂载上下文 |

### 怎么理解

可以把它理解成流水线上的一个工位：

- `dispatch()` 是开始加工
- `done()` 是加工完成并交接下一个工位

## `SeriesWork`

文件：`src/factory/Workflow.h`、`src/factory/Workflow.cc`

表示一个串行工作流。

### 关键函数

| 函数 | 输入 | 输出 | 作用 |
|---|---|---|---|
| `start()` | 无 | 无 | 从第一个任务开始启动 |
| `dismiss()` | 无 | 无 | 放弃整个未启动流程 |
| `push_back(SubTask *task)` | 新任务 | 无 | 加到队尾 |
| `push_front(SubTask *task)` | 新任务 | 无 | 插到队头 |
| `get_context()` | 无 | `void *` | 获取流程上下文 |
| `set_context(void *context)` | 上下文 | 无 | 设置上下文 |
| `cancel()` | 无 | 无 | 取消后续任务 |
| `is_canceled()` | 无 | `bool` | 是否取消 |
| `is_finished()` | 无 | `bool` | 是否结束 |
| `set_callback(...)` | series 回调 | 无 | 设置结束回调 |
| `pop()` | 无 | `SubTask *` | 取下一个任务 |

### 本质

`SeriesWork` 解决的是“先做 A，再做 B，再做 C”。

## `ParallelWork`

文件：`src/factory/Workflow.h`、`src/factory/Workflow.cc`

表示多个串行流并发执行。

### 关键函数

| 函数 | 输入 | 输出 | 作用 |
|---|---|---|---|
| `start()` | 无 | 无 | 启动并行组 |
| `dismiss()` | 无 | 无 | 放弃并行组 |
| `add_series(SeriesWork *series)` | 子流程 | 无 | 加入一个子流程 |
| `get_context()` | 无 | `void *` | 获取上下文 |
| `set_context(void *context)` | 上下文 | 无 | 设置上下文 |
| `series_at(size_t index)` | 下标 | `SeriesWork *` | 获取指定子流程 |
| `size()` | 无 | `size_t` | 子流程数量 |

### 本质

`ParallelWork` 解决的是“同时做多个子流程，等全部完成后再继续”。

## `Workflow`

文件：`src/factory/Workflow.h`

它是创建和启动工作流的静态入口。

### 关键函数

| 函数 | 输入 | 输出 | 作用 |
|---|---|---|---|
| `create_series_work(SubTask *first, callback)` | 第一个任务、回调 | `SeriesWork *` | 创建串行流 |
| `start_series_work(SubTask *first, callback)` | 第一个任务、回调 | 无 | 创建并直接启动串行流 |
| `create_parallel_work(callback)` | 回调 | `ParallelWork *` | 创建空并行流 |
| `create_parallel_work(SeriesWork *const all_series[], size_t n, callback)` | 子流程数组、数量、回调 | `ParallelWork *` | 创建并行流 |
| `start_parallel_work(...)` | 同上 | 无 | 创建并直接启动并行流 |

## 面试里的讲法

这个项目最大的抽象价值在于，它没有把 HTTP、Redis、定时器、文件 IO 当成完全不同的东西，而是先统一成 `SubTask`，再通过 `SeriesWork` 和 `ParallelWork` 去表达顺序关系和并发关系。这让不同类型的异步能力都能放进同一个工作流中。
