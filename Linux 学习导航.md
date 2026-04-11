# Linux 学习导航

## 当前课程

- [[linux系统编程/系统编程笔记]]
- [[linux网络编程/网络编程笔记]]

## 建议学习顺序

1. 先完成 `Linux 系统编程` 前 5 个模块，建立文件 IO 和进程基础
2. 再学习 `信号`、`IPC`、`线程`，把并发基础补齐
3. 然后进入 `Linux 网络编程`，先写最简单的 TCP 客户端/服务器
4. 最后学习 `select/poll/epoll`、线程池和 `libevent`

## 笔记组织建议

### 目录建议

```text
计算机学习/
├─ Linux 学习导航.md
├─ attachments/
├─ linux系统编程/
│  └─ 系统编程笔记.md
└─ linux网络编程/
   └─ 网络编程笔记.md
```

### 使用方式

- 每看完一节视频，就在对应课程页勾掉一个任务
- 课堂代码建议单独建代码仓库，不直接堆进笔记正文
- 截图、流程图、抓包结果统一放进 `attachments`
- 每个知识点都补一段“我自己的理解”和一个最小示例

## 两门课之间的关系

- 系统编程解决“进程、线程、资源、同步”的问题
- 网络编程解决“连接、通信、并发、事件驱动”的问题
- 真正的服务器开发通常是两者结合：`socket + 进程/线程 + 同步 + IO 多路复用`

## 附件目录

- `attachments/` 已创建，可直接用于 Obsidian 引用

## 来源

- 系统编程视频：[BV1KE411q7ee](https://www.bilibili.com/video/BV1KE411q7ee)
- 网络编程视频：[BV1iJ411S7UA](https://www.bilibili.com/video/BV1iJ411S7UA)
- 系统编程课件：[语雀链接](https://www.yuque.com/boyhui/ukcpwf/mo9pk8qbgk0a7dtz)
- 网络编程课件：[语雀链接](https://www.yuque.com/boyhui/ukcpwf/ieg4t7k1d32hvbvb)
