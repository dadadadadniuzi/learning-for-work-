---
title: poll模型
tags:
  - linux
  - 网络编程
  - 概念词条
---
# poll模型

## 它是什么

- poll 模型是使用 `pollfd` 数组监听多个 fd 的 IO 多路复用模型。

## 工作流程

- 准备 `pollfd` 数组。
- 每个元素设置一个 fd 和关心的事件。
- 调用 `poll` 等待事件。
- 遍历数组检查 `revents`。

## 优点

- 不需要传最大 fd 加 1。
- 不使用 `fd_set`，避免固定大小位图限制。
- `events` 和 `revents` 分离，语义更清晰。

## 缺点

- 每次调用仍然要把数组交给内核。
- 返回后仍然需要线性遍历数组。

## 常见出现位置

- [[linux网络编程/课时笔记/05 IO多路复用/02 poll|02 poll]]
- [[linux网络编程/函数笔记/IO多路复用/poll|poll]]
