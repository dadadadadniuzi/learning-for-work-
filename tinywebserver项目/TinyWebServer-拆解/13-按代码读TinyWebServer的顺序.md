---
title: 按代码读TinyWebServer的顺序
aliases:
  - TinyWebServer 代码阅读路线
tags:
  - project
  - interview
  - code-reading
created: 2026-04-26
---

# 按代码读TinyWebServer的顺序

返回：[[TinyWebServer-面试拆解笔记]]

相关：[[TinyWebServer-10分钟总复盘版]]、[[TinyWebServer-拆解/03-入口与WebServer总控]]、[[TinyWebServer-拆解/04-http_conn与HTTP状态机]]、[[TinyWebServer-拆解/12-半同步半反应堆与并发模型详解]]

> [!abstract]
> 这篇是“直接进代码”时的路线图。目标不是第一遍把所有实现细节吃完，而是先抓主干，再补模块。

## 最推荐的阅读顺序

1. `TinyWebServer-master/main.cpp`
2. `TinyWebServer-master/config.h`
3. `TinyWebServer-master/config.cpp`
4. `TinyWebServer-master/webserver.h`
5. `TinyWebServer-master/webserver.cpp`
6. `TinyWebServer-master/http/http_conn.h`
7. `TinyWebServer-master/http/http_conn.cpp`
8. `TinyWebServer-master/threadpool/threadpool.h`
9. `TinyWebServer-master/timer/lst_timer.h`
10. `TinyWebServer-master/timer/lst_timer.cpp`
11. `TinyWebServer-master/CGImysql/sql_connection_pool.h`
12. `TinyWebServer-master/CGImysql/sql_connection_pool.cpp`
13. `TinyWebServer-master/log/log.h`
14. `TinyWebServer-master/log/log.cpp`
15. `TinyWebServer-master/log/block_queue.h`
16. `TinyWebServer-master/lock/locker.h`
17. `TinyWebServer-master/root/`

## 为什么这样读

这个顺序符合“从总控到细节、从主流程到支撑模块”的阅读方式：

- 先看服务器怎么启动
- 再看主线程怎么监听和分发事件
- 再看单连接怎么处理 HTTP
- 最后补线程池、定时器、数据库池、日志这些支撑模块

这样最不容易迷路。

## 第一轮只读什么

第一轮只看这 7 个文件：

1. `TinyWebServer-master/main.cpp`
2. `TinyWebServer-master/config.cpp`
3. `TinyWebServer-master/webserver.h`
4. `TinyWebServer-master/webserver.cpp`
5. `TinyWebServer-master/http/http_conn.h`
6. `TinyWebServer-master/http/http_conn.cpp`
7. `TinyWebServer-master/threadpool/threadpool.h`

### 第一轮目标

- 程序启动顺序是什么
- 主线程负责什么
- 工作线程负责什么
- 单个请求怎么从读到写跑完
- Reactor / Proactor 在代码里从哪分叉

## 第二轮读什么

第二轮再看：

1. `TinyWebServer-master/timer/lst_timer.cpp`
2. `TinyWebServer-master/CGImysql/sql_connection_pool.cpp`
3. `TinyWebServer-master/log/log.cpp`
4. `TinyWebServer-master/log/block_queue.h`
5. `TinyWebServer-master/lock/locker.h`

### 第二轮目标

- 为什么要定时器
- 为什么要数据库连接池
- 为什么要异步日志
- 锁、条件变量、信号量各服务于哪里

## 第三轮再补什么

最后补：

- `TinyWebServer-master/root/`

这个目录能帮你把“页面跳转”和“URL 映射”对上。

## 每个核心文件带着什么问题去读

### `main.cpp`

- 初始化顺序是什么
- 哪一步进入主循环

### `webserver.cpp`

- 主线程核心逻辑在哪里
- `eventListen()` 做了哪些系统调用
- `eventLoop()` 如何分发连接、读写、信号、超时

### `http_conn.cpp`

- HTTP 状态机怎么拆
- GET 和 POST 怎么区分
- 登录注册逻辑写在哪
- 响应如何返回

### `threadpool.h`

- 工作线程如何等待任务
- 任务如何入队出队
- Reactor / Proactor 如何体现

### `timer/lst_timer.cpp`

- 连接什么时候绑定定时器
- 活动连接如何延长过期时间

### `sql_connection_pool.cpp`

- 连接怎么借和还
- RAII 在哪里用到了

### `log/log.cpp`

- 同步日志和异步日志从哪里分叉
- 阻塞队列在这里扮演什么角色

## 最实用的读法

### 第一步

先在纸上写一条主链：

```text
main
 -> parse_arg
 -> server.init
 -> log_write
 -> sql_pool
 -> thread_pool
 -> trig_mode
 -> eventListen
 -> eventLoop
```

### 第二步

把 `eventLoop()` 当成主线程大脑来读，弄清楚：

- 新连接怎么进来
- 读写事件怎么分出去
- 定时器和信号怎么接进来

### 第三步

把 `http_conn::process()` 当成单连接主链来读，顺着看：

```text
read_once
 -> process_read
 -> do_request
 -> process_write
 -> write
```

### 第四步

最后再回头看线程池、定时器、数据库池和日志，你会更容易知道它们分别在给谁服务。

## 我这次已经帮你补了哪些注释

我在这些关键文件里加了阅读型注释：

- `TinyWebServer-master/main.cpp`
- `TinyWebServer-master/webserver.cpp`
- `TinyWebServer-master/http/http_conn.cpp`
- `TinyWebServer-master/threadpool/threadpool.h`

这些注释重点解释的是：

- 为什么这个函数重要
- 它在主流程里扮演什么角色
- Reactor / Proactor 在哪分开
- 状态机和线程池分别在解决什么问题

## 现在最适合你的阅读节奏

### 如果只剩 30 分钟

只读：

1. `TinyWebServer-master/main.cpp`
2. `TinyWebServer-master/webserver.cpp`
3. `TinyWebServer-master/http/http_conn.cpp`

### 如果有 1 小时

再加上：

4. `TinyWebServer-master/threadpool/threadpool.h`
5. `TinyWebServer-master/timer/lst_timer.cpp`
6. `TinyWebServer-master/CGImysql/sql_connection_pool.cpp`

### 如果有 2 小时

按这篇完整顺序全部过一遍。

## 最后一个建议

第一遍不要追求逐行精读。  
最重要的是先回答这 4 个问题：

1. 服务器怎么启动
2. 主线程怎么分发事件
3. 单连接怎么处理 HTTP
4. 线程池和定时器怎么协作

只要这 4 个问题顺了，你后面再补细节会轻松很多。

