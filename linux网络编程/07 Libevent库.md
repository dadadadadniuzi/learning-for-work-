---
title: 07 Libevent库
tags:
  - linux
  - 网络编程
  - 模块复习
  - 网络编程/Libevent
---
# 07 Libevent库

## 本章目标

- 理解 [[linux网络编程/概念词条/Libevent|Libevent]] 是对底层 [[linux网络编程/概念词条/IO多路复用|IO多路复用]] 和事件循环的封装。
- 理解 [[linux网络编程/概念词条/Reactor反应堆模式|Reactor（反应堆模式）]] 与 Libevent 的关系：事件循环等待事件，事件就绪后调用回调。
- 掌握 [[linux网络编程/概念词条/event_base|event_base]]、[[linux网络编程/概念词条/event|event]]、[[linux网络编程/概念词条/bufferevent|bufferevent]]、[[linux网络编程/概念词条/evconnlistener|evconnlistener]] 的分工。
- 学会普通 `event` 的创建、添加、事件循环和释放流程。
- 学会 `bufferevent` 的创建、回调设置、读写使能、读写数据和释放流程。
- 理解 [[linux网络编程/概念词条/事件的未决与非未决|事件的未决与非未决]]，知道为什么 `event_new` 后还必须 `event_add`。
- 能把 Libevent 服务端流程和前面的 [[linux网络编程/05 IO多路复用|05 IO多路复用]]、[[linux网络编程/03 TCP通信与通信案例|03 TCP通信与通信案例]] 对应起来。

## 核心函数

- [[linux网络编程/函数笔记/Libevent/event_base_new|event_base_new]]
- [[linux网络编程/函数笔记/Libevent/event_base_free|event_base_free]]
- [[linux网络编程/函数笔记/Libevent/event_new|event_new]]
- [[linux网络编程/函数笔记/Libevent/evsignal_new|evsignal_new]]
- [[linux网络编程/函数笔记/Libevent/event_add|event_add]]
- [[linux网络编程/函数笔记/Libevent/event_free|event_free]]
- [[linux网络编程/函数笔记/Libevent/event_base_dispatch|event_base_dispatch]]
- [[linux网络编程/函数笔记/Libevent/bufferevent_socket_new|bufferevent_socket_new]]
- [[linux网络编程/函数笔记/Libevent/bufferevent_setcb|bufferevent_setcb]]
- [[linux网络编程/函数笔记/Libevent/bufferevent_enable|bufferevent_enable]]
- [[linux网络编程/函数笔记/Libevent/bufferevent_disable|bufferevent_disable]]
- [[linux网络编程/函数笔记/Libevent/bufferevent_read|bufferevent_read]]
- [[linux网络编程/函数笔记/Libevent/bufferevent_write|bufferevent_write]]
- [[linux网络编程/函数笔记/Libevent/bufferevent_socket_connect|bufferevent_socket_connect]]
- [[linux网络编程/函数笔记/Libevent/bufferevent_free|bufferevent_free]]
- [[linux网络编程/函数笔记/Libevent/evconnlistener_new_bind|evconnlistener_new_bind]]
- [[linux网络编程/函数笔记/Libevent/evconnlistener_free|evconnlistener_free]]

## 本模块课时

- [[linux网络编程/课时笔记/07 Libevent库/01 Libevent简介与安装|01 Libevent简介与安装]]
- [[linux网络编程/课时笔记/07 Libevent库/02 常规event基础|02 常规event基础]]
- [[linux网络编程/课时笔记/07 Libevent库/03 event实现本地通信|03 event实现本地通信]]
- [[linux网络编程/课时笔记/07 Libevent库/04 事件的未决与非未决|04 事件的未决与非未决]]
- [[linux网络编程/课时笔记/07 Libevent库/05 bufferevent基础|05 bufferevent基础]]
- [[linux网络编程/课时笔记/07 Libevent库/06 evconnlistener与通信流程|06 evconnlistener与通信流程]]

## 本模块概念

- [[linux网络编程/概念词条/Libevent|Libevent]]
- [[linux网络编程/概念词条/Reactor反应堆模式|Reactor（反应堆模式）]]
- [[linux网络编程/概念词条/event_base|event_base]]
- [[linux网络编程/概念词条/event|event]]
- [[linux网络编程/概念词条/event_callback_fn|event_callback_fn]]
- [[linux网络编程/概念词条/事件回调函数|事件回调函数]]
- [[linux网络编程/概念词条/Libevent事件标志|Libevent事件标志]]
- [[linux网络编程/概念词条/事件的未决与非未决|事件的未决与非未决]]
- [[linux网络编程/概念词条/evutil_socket_t|evutil_socket_t]]
- [[linux网络编程/概念词条/struct timeval|struct timeval]]
- [[linux网络编程/概念词条/bufferevent|bufferevent]]
- [[linux网络编程/概念词条/bufferevent回调|bufferevent回调]]
- [[linux网络编程/概念词条/bufferevent_data_cb|bufferevent_data_cb]]
- [[linux网络编程/概念词条/bufferevent_event_cb|bufferevent_event_cb]]
- [[linux网络编程/概念词条/bufferevent读写缓冲区|bufferevent读写缓冲区]]
- [[linux网络编程/概念词条/BEV_OPT_CLOSE_ON_FREE|BEV_OPT_CLOSE_ON_FREE]]
- [[linux网络编程/概念词条/BEV_EVENT_CONNECTED|BEV_EVENT_CONNECTED]]
- [[linux网络编程/概念词条/BEV_EVENT_EOF|BEV_EVENT_EOF]]
- [[linux网络编程/概念词条/BEV_EVENT_ERROR|BEV_EVENT_ERROR]]
- [[linux网络编程/概念词条/evconnlistener|evconnlistener]]
- [[linux网络编程/概念词条/evconnlistener_cb|evconnlistener_cb]]
- [[linux网络编程/概念词条/LEV_OPT_CLOSE_ON_FREE与LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE 与 LEV_OPT_REUSEABLE]]
- [[linux网络编程/概念词条/Libevent连接流程|Libevent连接流程]]

## 本模块指令

- [[linux网络编程/指令查询/命令卡/tar|tar]]
- [[linux网络编程/指令查询/命令卡/make|make]]

## 细节补充

- Libevent 的普通事件流程是：先创建 [[linux网络编程/函数笔记/Libevent/event_base_new|event_base_new]]，再用 [[linux网络编程/函数笔记/Libevent/event_new|event_new]] 创建事件，用 [[linux网络编程/函数笔记/Libevent/event_add|event_add]] 加入监听，最后用 [[linux网络编程/函数笔记/Libevent/event_base_dispatch|event_base_dispatch]] 启动循环。
- Libevent 可以理解成 Reactor 风格的库：[[linux网络编程/概念词条/event_base|event_base]] 负责事件循环，事件就绪后调用 [[linux网络编程/概念词条/事件回调函数|事件回调函数]]。
- [[linux网络编程/概念词条/event|event]] 适合直接监听 fd 的读写事件；[[linux网络编程/概念词条/bufferevent|bufferevent]] 在 fd 上封装了读写缓冲区和回调，更适合 TCP 通信。
- [[linux网络编程/函数笔记/Libevent/evconnlistener_new_bind|evconnlistener_new_bind]] 帮服务端封装了传统 [[linux网络编程/函数笔记/Socket/socket|socket]]、[[linux网络编程/函数笔记/Socket/bind|bind]]、[[linux网络编程/函数笔记/Socket/listen|listen]]、[[linux网络编程/函数笔记/Socket/accept|accept]] 流程。
- 编译使用 Libevent 的程序时，通常需要链接 `-levent`。

## 复习路线

- 先理解 `event_base + event + event_base_dispatch`。
- 再把这个流程对应到 [[linux网络编程/概念词条/Reactor反应堆模式|Reactor（反应堆模式）]]：注册事件、等待事件、分发回调。
- 再理解事件状态：[[linux网络编程/概念词条/事件的未决与非未决|未决与非未决]]。
- 然后学习 [[linux网络编程/概念词条/bufferevent|bufferevent]] 的读写回调和缓冲区。
- 最后把 [[linux网络编程/概念词条/evconnlistener|evconnlistener]] 和 [[linux网络编程/概念词条/bufferevent|bufferevent]] 组合成完整服务端流程。
