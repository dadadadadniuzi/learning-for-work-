---
title: tcpdump
tags:
  - linux
  - 网络编程
  - 指令查询
---
# tcpdump

## 命令作用

- `tcpdump` 是命令行抓包工具，可以观察网卡上的网络数据包。
- 在网络编程学习中，它常用于验证连接建立、数据收发、端口过滤等现象。

## 常见写法

```bash
sudo tcpdump -i lo
sudo tcpdump -i lo tcp port 9527
sudo tcpdump -i any port 9527
sudo tcpdump -nn -i lo tcp
```

## 参数说明

- `-i 网卡`：指定抓包网卡。`lo` 是本机回环接口，`any` 表示尽量抓所有接口。
- `-n`：不把地址解析成主机名。
- `-nn`：地址和端口都用数字显示。
- `tcp`：只抓 TCP 包。
- `port 端口`：只抓指定端口相关数据包。

## 常见用途

- 观察 TCP 客户端和服务器是否真的有数据交互。
- 调试本机 `127.0.0.1` 通信时，可以抓 `lo` 网卡。
- 配合 `nc` 或自己写的 TCP 程序观察连接行为。

## 示例

```bash
sudo tcpdump -nn -i lo tcp port 9527
```

这条命令会在本机回环接口上抓取 TCP 端口 `9527` 的数据包，并以数字形式显示地址和端口。

## 易错点

- 抓包通常需要 root 权限，所以常加 `sudo`。
- 本机 `127.0.0.1` 通信走 `lo`，不是物理网卡。
- `tcpdump` 能看到包，不代表应用层一定正确处理了数据。

## 常见出现位置

- [[linux网络编程/05 IO多路复用|05 IO多路复用]]
- [[linux网络编程/03 TCP通信与通信案例|03 TCP通信与通信案例]]
