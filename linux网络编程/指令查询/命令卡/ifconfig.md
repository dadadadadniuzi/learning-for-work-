---
title: ifconfig
tags:
  - linux
  - 网络编程
  - 指令查询
---
# ifconfig

## 命令作用

- `ifconfig` 用于查看和配置网络接口。
- 在课程和很多旧资料中，经常用它查看本机 IP 地址、网卡名称和网络接口状态。

## 常见写法

```bash
ifconfig
ifconfig eth0
ifconfig lo
```

## 输出重点

- 网卡名称：例如 `eth0`、`ens33`、`lo`。
- `inet`：IPv4 地址。
- `netmask`：子网掩码。
- `broadcast`：广播地址。
- `UP`：表示网络接口处于启用状态。

## 常见用途

- 查看虚拟机或 Linux 主机的 IP 地址。
- 确认回环接口 `lo` 是否存在。
- 配合 socket 程序确定客户端连接的目标 IP。

## 和其他命令的关系

- 新系统可能默认不安装 `ifconfig`，可以使用 [[linux网络编程/指令查询/命令卡/ip|ip]] 替代。
- 查看连通性时常配合 [[linux网络编程/指令查询/命令卡/ping|ping]]。

## 常见出现位置

- [[linux网络编程/01 网络基础|01 网络基础]]
- [[linux网络编程/课时笔记/01 网络基础/02 七层模型和四层模型|02 七层模型和四层模型]]
