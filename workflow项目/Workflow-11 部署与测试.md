---
title: Workflow-11 部署与测试
tags:
  - workflow
  - deploy
  - test
  - interview
aliases:
  - Workflow 测试量化
  - Workflow 部署测试
---

# Workflow-11 部署与测试

相关笔记：
[[Workflow项目面试拆解]] | [[Workflow-01 项目定位与一句话介绍]] | [[Workflow-04 HTTP 请求与服务端链路]] | [[Workflow-09 高频面试题与背诵稿]] | [[Workflow-10 10分钟速背版]]

## 这一章回答什么

这一章回答两个很实际的问题：

- 这个项目是不是一个网站？
- 没有界面，怎么证明自己真的学过、跑过、测试过？

## 先明确：它不是网站项目

`workflow-master` 不是一个可以直接部署上线的网站。

它没有：

- 前端页面
- 登录系统
- 后台管理界面
- 业务数据库模型
- 现成的网站路由和业务接口

它本质上是一个 **C++ 异步网络工作流框架**。

更准确地说，它是用来写高性能后端服务的基础设施。它可以帮你开发 HTTP Server、HTTP Client、Redis/MySQL/Kafka 客户端、自定义协议服务、异步任务流、微服务和本地负载均衡能力。

## 能不能直接部署

不能按“网站项目”的方式直接部署。

也就是说，它不是这种项目：

```bash
npm run build
docker compose up
```

它更像一个 C++ 库和后端框架。你需要先编译它，再基于它写自己的服务程序。

官方 README 里的快速开始是：

```bash
git clone https://github.com/sogou/workflow
cd workflow
make
cd tutorial
make
```

这说明它默认做的是：

- 编译框架
- 编译 tutorial 示例
- 让你运行示例程序理解框架能力

## 怎么测试它的效果

虽然它没有界面，但它非常适合用后端指标来量化。

可以从这几个角度测试：

- 服务是否能启动
- HTTP 请求是否能正常响应
- 并发下 QPS 有多少
- 平均延迟和 P99 延迟是多少
- 高并发下错误率是多少
- 串行任务是否按顺序执行
- 并行任务是否真的并发执行
- DNS、Timer、File IO、Upstream 是否能按预期工作

## 第 1 步：跑通 HTTP Server 示例

先编译项目和示例：

```bash
cd workflow-master
make
cd tutorial
make
```

运行最简单的 HTTP server：

```bash
./tutorial-00-helloworld
```

然后用 `curl` 访问：

```bash
curl http://127.0.0.1:8888
```

如果返回类似 `Hello World` 的内容，说明你已经跑通了一个基于 Workflow 的 HTTP 服务。

## 第 2 步：用压测工具量化

如果有 `wrk`，可以这样压测：

```bash
wrk -t4 -c100 -d30s http://127.0.0.1:8888/
```

重点记录：

| 指标 | 含义 |
|---|---|
| Requests/sec | 每秒处理多少请求，也就是 QPS |
| Latency | 平均延迟 |
| P99 Latency | 99% 请求的延迟上限 |
| Transfer/sec | 每秒传输数据量 |
| Non-2xx / Error | 失败请求数量 |

如果没有 `wrk`，也可以用 `ab`：

```bash
ab -n 10000 -c 100 http://127.0.0.1:8888/
```

## 第 3 步：做几个小实验

### HTTP Server 实验

目标：验证服务端封装能力。

可以写一个 echo server：

- 收到请求
- 读取 path 或 body
- 返回固定响应或原样返回

量化指标：

- QPS
- 平均延迟
- 并发连接数
- 错误率

### HTTP Client 实验

目标：验证异步客户端能力。

可以用 `WFTaskFactory::create_http_task()` 同时请求多个 URL。

量化指标：

- 总耗时
- 成功率
- 失败 URL 数量
- callback 触发次数

### SeriesWork 实验

目标：验证串行工作流。

设计一个流程：

```text
HTTP 请求 -> Timer 等待 -> 写文件 -> callback 汇总
```

量化指标：

- 执行顺序是否正确
- 每一步是否在前一步结束后才开始
- 最终 callback 是否只触发一次

### ParallelWork 实验

目标：验证并行工作流。

设计一个流程：

```text
同时发起 10 个 HTTP 请求 -> 全部完成 -> 汇总结果
```

量化指标：

- 总耗时是否接近最慢的那个任务
- 是否明显快于串行执行
- 每个子任务是否都完成

### TimerTask 实验

目标：验证定时器任务。

可以创建 1 秒、2 秒、5 秒的定时器。

量化指标：

- 实际触发时间
- 和预期时间的误差
- 多个定时器是否互不阻塞

### File IO 实验

目标：验证异步文件读写。

可以用 `create_pread_task()` 和 `create_pwrite_task()`。

量化指标：

- 读写耗时
- 文件大小
- 成功率
- callback 是否正常触发

### DNS 实验

目标：验证 DNS 解析和缓存。

可以重复解析同一个域名。

量化指标：

- 第一次解析耗时
- 缓存命中后耗时
- 解析失败时的错误状态

### Upstream 实验

目标：验证本地负载均衡和失败切换。

可以配置多个后端地址，然后请求同一个 upstream 名称。

量化指标：

- 请求分布比例
- 某个后端失败后是否切换
- 全部后端失败时错误码

## 推荐量化表

| 测试项 | 怎么测 | 量化指标 |
|---|---|---|
| HTTP Server | `curl` + `wrk` | QPS、延迟、失败率 |
| HTTP Client | 并发请求多个 URL | 完成时间、成功率 |
| SeriesWork | A 后接 B 后接 C | 执行顺序是否正确 |
| ParallelWork | 同时跑多个任务 | 总耗时是否接近最慢任务 |
| TimerTask | 定时触发 | 误差毫秒数 |
| FileTask | 异步读写文件 | 吞吐、耗时 |
| DNS | 重复解析域名 | 首次耗时、缓存后耗时 |
| Upstream | 多后端选择 | 分流比例、失败切换 |

## 面试里怎么讲

如果面试官问：“这个项目没有界面，你怎么验证自己学过？”

可以这样回答：

我没有把它当成网站项目来测试，而是按后端框架来验证。首先我编译了 `workflow-master` 和 tutorial 示例，跑通了 `WFHttpServer` 的 helloworld 服务，并用 `curl` 验证 HTTP 响应。然后可以用 `wrk` 或 `ab` 对服务进行压测，观察 QPS、延迟和错误率。除此之外，我还可以分别写小实验验证 `SeriesWork` 的串行编排、`ParallelWork` 的并行编排、`TimerTask` 的定时能力、File IO 的异步读写、DNS 缓存以及 Upstream 的负载均衡和失败切换。

## 最后总结

这个项目不能像普通网站一样直接部署，但它可以被编译成 C++ 后端框架，并用它开发可部署的 HTTP 服务或微服务。

学习它时，最好的量化方式不是看有没有页面，而是看：

- 能不能跑通服务
- 能不能压测出 QPS 和延迟
- 能不能解释请求链路
- 能不能用实验验证任务编排
- 能不能说清楚异步 IO、DNS、路由、Upstream、错误处理和超时机制

一句话：

**Workflow 没有界面，但它的学习成果可以通过服务运行、并发压测、任务实验和源码链路解释来量化。**
