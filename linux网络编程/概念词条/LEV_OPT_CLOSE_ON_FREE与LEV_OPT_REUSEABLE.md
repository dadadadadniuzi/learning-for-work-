---
title: LEV_OPT_CLOSE_ON_FREE 与 LEV_OPT_REUSEABLE
tags:
  - linux
  - 网络编程
  - 概念词条
  - 网络编程/Libevent
---
# LEV_OPT_CLOSE_ON_FREE 与 LEV_OPT_REUSEABLE

这两个宏是创建 [[linux网络编程/概念词条/evconnlistener|evconnlistener]] 时常用的 listener 选项。

## LEV_OPT_CLOSE_ON_FREE

释放 listener 时自动关闭底层监听 socket。它能减少忘记 `close` 导致的资源泄漏。

## LEV_OPT_REUSEABLE

允许地址复用，效果类似传统 socket 编程中设置 [[linux网络编程/概念词条/SO_REUSEADDR|SO_REUSEADDR]]。服务器重启时，端口不容易因为上一次连接状态而绑定失败。

## 常见组合

```c
LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE
```

## 易错点

- 组合多个选项要用按位或 `|`，不是逻辑或 `||`。
- 它们是 listener 选项，不是 bufferevent 选项；bufferevent 常见的是 [[linux网络编程/概念词条/BEV_OPT_CLOSE_ON_FREE|BEV_OPT_CLOSE_ON_FREE]]。

## 相关函数

- [[linux网络编程/函数笔记/Libevent/evconnlistener_new_bind|evconnlistener_new_bind]]
- [[linux网络编程/函数笔记/Libevent/evconnlistener_free|evconnlistener_free]]

## 相关模块

- [[linux网络编程/07 Libevent库|07 Libevent库]]
