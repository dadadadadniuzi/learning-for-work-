---
title: struct linger
tags:
  - linux
  - 网络编程
  - 概念词条
---
# struct linger

## 是什么

`struct linger` 是设置 [[linux网络编程/概念词条/SO_LINGER|SO_LINGER]] 时使用的结构体。

## 常见定义

```c
struct linger {
    int l_onoff;
    int l_linger;
};
```

## 依赖头文件

- `#include <sys/socket.h>`

## 字段解释

- `l_onoff`
  是否启用 `SO_LINGER`。
  - `0`：不启用
  - `1`：启用

- `l_linger`
  linger 等待时间，单位通常按秒理解。
  只有在 `l_onoff = 1` 时它才有实际意义。

## 怎么理解

它本质上是在回答两个问题：

- 要不要接管关闭行为
- 如果要接管，最多等多久

## 常见组合

- `l_onoff = 0`
  使用默认关闭行为

- `l_onoff = 1, l_linger = 5`
  关闭时最多等 5 秒处理剩余数据

- `l_onoff = 1, l_linger = 0`
  立即关闭，通常更偏向强制中断

## 相关笔记

- [[linux网络编程/概念词条/SO_LINGER|SO_LINGER]]
- [[linux网络编程/概念词条/SOL_SOCKET|SOL_SOCKET]]
- [[linux网络编程/函数笔记/Socket/setsockopt|setsockopt]]
