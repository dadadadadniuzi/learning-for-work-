---
title: Libevent简介与安装
tags:
  - linux
  - 网络编程
  - 课时笔记
  - 网络编程/Libevent
---
# Libevent简介与安装

## 本节学什么

- [[linux网络编程/概念词条/Libevent|Libevent]] 是什么
- [[linux网络编程/概念词条/Reactor反应堆模式|Reactor（反应堆模式）]]
- Libevent 解决什么问题
- 安装流程
- 编译链接方式

## 本节学什么详解

- [[linux网络编程/概念词条/Libevent|Libevent]]：一个事件驱动网络库，封装了 `select/poll/epoll` 等底层 IO 多路复用机制，让程序用回调函数处理事件。
- [[linux网络编程/概念词条/Reactor反应堆模式|Reactor（反应堆模式）]]：事件驱动服务器的常见组织方式，负责等待事件、分发事件并调用处理函数；Libevent 就是这种思想的典型封装。
- 解决什么问题：前面手写 `select/poll/epoll` 时需要自己维护 fd 集合、事件数组和循环，Libevent 把这些通用逻辑封装起来。
- 安装流程：源码包通常通过 `tar` 解压，然后执行 `./configure`、`make`、`sudo make install`。
- 编译链接方式：包含对应头文件，并在编译时链接 `-levent`。

## 知识点补充

- Libevent 是“事件循环 + 事件对象 + 回调函数”的编程模式。
- 这个模式可以从 [[linux网络编程/概念词条/Reactor反应堆模式|Reactor（反应堆模式）]] 角度理解：事件循环负责等，回调函数负责处理。
- 学 Libevent 前要先理解 [[linux网络编程/概念词条/IO多路复用|IO多路复用]] 和 [[linux网络编程/概念词条/事件就绪|事件就绪]]。
- 编译失败时要区分头文件找不到、库文件找不到和链接参数缺失。

## 本节内容速览

- 下载或准备源码包。
- 用 `tar` 解压。
- `./configure` 检查环境并生成 Makefile。
- `make` 编译。
- `sudo make install` 安装。
- 编译自己的程序时加 `-levent`。

## 复习时要回答

- Libevent 相比手写 epoll 帮我们封装了什么？
- 为什么编译 Libevent 程序要加 `-levent`？
- `tar`、`make` 在安装流程里分别做什么？

## 本节关键指令

- [[linux网络编程/指令查询/命令卡/tar|tar]]
- [[linux网络编程/指令查询/命令卡/make|make]]

## 本节关键概念

- [[linux网络编程/概念词条/Libevent|Libevent]]
- [[linux网络编程/概念词条/Reactor反应堆模式|Reactor（反应堆模式）]]
- [[linux网络编程/概念词条/事件回调函数|事件回调函数]]

## 关联模块

- [[linux网络编程/07 Libevent库|07 Libevent库]]
