---
title: itimerval结构
tags:
  - linux
  - 系统编程
  - 概念词条
  - 信号
---
# itimerval结构

## 它是什么
- `setitimer` 使用的定时器配置结构体。

## 主要成员
- `it_value`：第一次触发前的时间。
- `it_interval`：后续周期触发间隔。
	-如果非 0，则之后每隔这么久触发一次

## 怎么理解
- `it_value` 像“启动倒计时”。
- `it_interval` 像“重复间隔”。
- 两者结合起来就能表示一次性定时或周期定时。

## 相关入口
- [[linux系统编程/函数笔记/信号/setitimer|setitimer]]
- [[linux系统编程/概念词条/定时器|定时器]]
