---
title: Workflow-06 kernel 层：Communicator 与 CommScheduler
tags:
  - workflow
  - kernel
  - network
  - interview
---

# Workflow-06 kernel 层：Communicator 与 CommScheduler

相关笔记：
[[Workflow项目面试拆解]] | [[Workflow-02 仓库结构与模块地图]] | [[Workflow-05 核心抽象：SubTask、SeriesWork、ParallelWork、Workflow]] | [[Workflow-08 路由、DNS、协议层]]

## 为什么这一层重要

如果说 `WFTaskFactory` 是你平时最常看到的接口，那么 `Communicator` 和 `CommScheduler` 就是整个框架真正的“发动机”。

## `Communicator`

文件：`src/kernel/Communicator.h`

它是底层通信引擎，负责真正的异步 IO 处理。

### 它要解决的问题

- 建连
- 发包
- 收包
- 超时
- 连接复用
- 服务监听
- 睡眠定时器
- 文件 IO 服务接入

### 相关核心类

- `CommTarget`：远端目标
- `CommSession`：一次通信会话
- `CommService`：一个服务监听体
- `SleepSession`：一个定时等待对象
- `IOService`：文件 IO 服务

### `Communicator` 核心函数

| 函数 | 输入 | 输出 | 作用 |
|---|---|---|---|
| `init(size_t poller_threads, size_t handler_threads)` | poller 线程数、handler 线程数 | `int` | 初始化通信引擎 |
| `deinit()` | 无 | 无 | 销毁引擎 |
| `request(CommSession *session, CommTarget *target)` | 会话、目标 | `int` | 发起客户端请求 |
| `reply(CommSession *session)` | 会话 | `int` | 回写服务端响应 |
| `push(const void *buf, size_t size, CommSession *session)` | 数据、大小、会话 | `int` | 直接推送数据 |
| `shutdown(CommSession *session)` | 会话 | `int` | 关闭会话 |
| `bind(CommService *service)` | 服务 | `int` | 绑定服务监听 |
| `unbind(CommService *service)` | 服务 | 无 | 取消监听 |
| `sleep(SleepSession *session)` | 定时会话 | `int` | 注册定时等待 |
| `unsleep(SleepSession *session)` | 定时会话 | `int` | 取消定时等待 |
| `io_bind(IOService *service)` | IO 服务 | `int` | 绑定文件 IO |
| `io_unbind(IOService *service)` | IO 服务 | 无 | 解绑文件 IO |

## `CommScheduler`

文件：`src/kernel/CommScheduler.h`

它在 `Communicator` 上面再包了一层调度逻辑。

### 它解决的问题

- 某个请求应该交给哪个目标
- 某个目标当前负载多少
- 当前是否还能分配连接

### 相关类

- `CommSchedObject`
- `CommSchedTarget`
- `CommSchedGroup`
- `CommScheduler`

### `CommSchedObject`

| 函数 | 输入 | 输出 | 作用 |
|---|---|---|---|
| `get_max_load() const` | 无 | `size_t` | 最大负载 |
| `get_cur_load() const` | 无 | `size_t` | 当前负载 |
| `acquire(int wait_timeout)` | 等待超时 | `CommTarget *` | 获取可用目标 |

### `CommSchedTarget`

| 函数 | 输入 | 输出 | 作用 |
|---|---|---|---|
| `init(...)` | 地址、超时、最大连接数 | `int` | 初始化单个目标 |
| `deinit()` | 无 | 无 | 销毁目标 |
| `acquire(int wait_timeout)` | 等待超时 | `CommTarget *` | 获取目标使用权 |
| `release()` | 无 | 无 | 归还使用权 |

### `CommSchedGroup`

| 函数 | 输入 | 输出 | 作用 |
|---|---|---|---|
| `init()` | 无 | `int` | 初始化目标组 |
| `deinit()` | 无 | 无 | 销毁目标组 |
| `add(CommSchedTarget *target)` | 目标 | `int` | 加入目标 |
| `remove(CommSchedTarget *target)` | 目标 | `int` | 移除目标 |
| `acquire(int wait_timeout)` | 等待超时 | `CommTarget *` | 从组里选一个可用目标 |

### `CommScheduler`

| 函数 | 输入 | 输出 | 作用 |
|---|---|---|---|
| `init(size_t poller_threads, size_t handler_threads)` | 线程参数 | `int` | 初始化调度器 |
| `request(...)` | session、object、wait_timeout、target 输出 | `int` | 发起请求并完成目标选择 |
| `reply(CommSession *session)` | 会话 | `int` | 回复请求 |
| `push(...)` | 数据、大小、会话 | `int` | 推送数据 |
| `bind(CommService *service)` | 服务 | `int` | 绑定服务 |
| `sleep(SleepSession *session)` | 定时会话 | `int` | 注册睡眠 |
| `io_bind(IOService *service)` | IO 服务 | `int` | 绑定文件 IO |

## 面试里的总结说法

`Communicator` 解决的是“怎么高效地做异步 IO”，而 `CommScheduler` 解决的是“请求到底分配给谁、连接怎么管、负载怎么控”。前者更像通信引擎，后者更像调度外壳。
