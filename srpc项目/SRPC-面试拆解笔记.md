---
title: SRPC 面试拆解笔记
tags:
  - interview
  - rpc
  - cpp
  - srpc
aliases:
  - SRPC 项目详解
  - srpc-master 面试笔记
---

# SRPC 面试拆解笔记

> [!abstract]
> 这是总目录页。下面的内容已经拆成多篇 Obsidian 笔记，方便你按主题复习、来回跳转和做面试前冲刺。

## 阅读顺序

1. [[SRPC-面试拆解/01-项目总览]]
2. [[SRPC-面试拆解/02-快速入口与示例]]
3. [[SRPC-面试拆解/03-从零实现路径]]
4. [[SRPC-面试拆解/04-请求完整链路]]
5. [[SRPC-面试拆解/05-核心模块-基础抽象]]
6. [[SRPC-面试拆解/06-核心模块-消息与任务]]
7. [[SRPC-面试拆解/07-核心模块-扩展与生成器]]
8. [[SRPC-面试拆解/08-面试话术与追问]]

## 按用途跳转

- 想先知道“这个项目到底是干什么的”：[[SRPC-面试拆解/01-项目总览]]
- 想先看 demo，快速建立感觉：[[SRPC-面试拆解/02-快速入口与示例]]
- 想讲“如果从零实现这个项目”：[[SRPC-面试拆解/03-从零实现路径]]
- 想讲一次 RPC 是怎么跑通的：[[SRPC-面试拆解/04-请求完整链路]]
- 想背核心类和接口：[[SRPC-面试拆解/05-核心模块-基础抽象]]
- 想背消息编解码和任务模型：[[SRPC-面试拆解/06-核心模块-消息与任务]]
- 想背 trace、metrics、代码生成：[[SRPC-面试拆解/07-核心模块-扩展与生成器]]
- 想直接练面试回答：[[SRPC-面试拆解/08-面试话术与追问]]

## 今晚最小复习集

如果时间特别紧，只看这四篇：

1. [[SRPC-面试拆解/01-项目总览]]
2. [[SRPC-面试拆解/03-从零实现路径]]
3. [[SRPC-面试拆解/04-请求完整链路]]
4. [[SRPC-面试拆解/08-面试话术与追问]]

## 源码入口

- [tutorial-01-srpc_pb_server.cc](G:\计算机学习\srpc项目\srpc-master\tutorial\tutorial-01-srpc_pb_server.cc)
- [tutorial-02-srpc_pb_client.cc](G:\计算机学习\srpc项目\srpc-master\tutorial\tutorial-02-srpc_pb_client.cc)
- [rpc_server.h](G:\计算机学习\srpc项目\srpc-master\src\rpc_server.h)
- [rpc_client.h](G:\计算机学习\srpc项目\srpc-master\src\rpc_client.h)
- [rpc_service.h](G:\计算机学习\srpc项目\srpc-master\src\rpc_service.h)
- [rpc_context.h](G:\计算机学习\srpc项目\srpc-master\src\rpc_context.h)
- [rpc_task.inl](G:\计算机学习\srpc项目\srpc-master\src\rpc_task.inl)
- [rpc_message_srpc.cc](G:\计算机学习\srpc项目\srpc-master\src\message\rpc_message_srpc.cc)
- [rpc_types.h](G:\计算机学习\srpc项目\srpc-master\src\rpc_types.h)
- [generator.cc](G:\计算机学习\srpc项目\srpc-master\src\generator\generator.cc)
