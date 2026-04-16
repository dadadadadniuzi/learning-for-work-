---
title: select模型
tags:
  - linux
  - 网络编程
  - 概念词条
---
# select模型

## 它是什么

- select 模型是早期经典的 IO 多路复用模型，通过 `fd_set` 集合让内核检查多个 fd 的就绪状态。

## 工作流程

- 准备读、写、异常三个 fd 集合。
- 调用 `select` 等待事件。
- `select` 返回后遍历 fd，使用 `FD_ISSET` 判断哪些 fd 就绪。
- 处理就绪 fd。

## 优点

- 接口经典，很多系统都支持。
- 适合学习 IO 多路复用基本思想。

## 缺点

- `fd_set` 有大小限制。
- 每次调用会修改集合，需要重复设置或复制。
- 每次返回后需要线性扫描 fd。

## 常见出现位置

- [[linux网络编程/课时笔记/05 IO多路复用/01 select|01 select]]
- [[linux网络编程/函数笔记/IO多路复用/select|select]]
