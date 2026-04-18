---
title: netstat
tags:
  - linux
  - 网络编程
  - 指令查询
---
# netstat

> [!info] 功能
> `netstat` 用于查看网络连接、监听端口、路由表、接口统计信息和 TCP/UDP socket 状态。它是旧资料和很多课程里常见的排查命令，新系统上常用 [[linux网络编程/指令查询/命令卡/ss|ss]] 替代。

## 基本格式

```bash
netstat [选项]
```

## 常用参数

- `-t`：显示 TCP 连接。
- `-u`：显示 UDP 连接。
- `-l`：只显示处于监听状态的 socket。
- `-a`：显示所有 socket，包括监听和非监听。
- `-n`：用数字显示 IP 和端口，不把端口解析成服务名。
- `-p`：显示进程 PID 和进程名，通常需要 root 权限。
- `-r`：显示路由表。
- `-i`：显示网络接口统计信息。
- `-s`：显示协议统计信息。

## 常见组合

```bash
netstat -tan
netstat -tln
netstat -tulpn
netstat -tanp
netstat -tanp | grep 9527
netstat -rn
```

## 常见组合解释

- `netstat -tan`：查看所有 TCP 连接和状态，常用于观察 `ESTABLISHED`、`TIME_WAIT`、`CLOSE_WAIT` 等。
- `netstat -tln`：查看 TCP 监听端口，常用于确认服务器是否真的执行了 `listen`。
- `netstat -tulpn`：查看 TCP/UDP 监听端口以及对应进程。
- `netstat -tanp | grep 9527`：筛选和端口 `9527` 相关的连接。
- `netstat -rn`：查看路由表，`-n` 避免解析域名或主机名。

## 输出字段怎么看

典型输出示例：

```text
Proto Recv-Q Send-Q Local Address     Foreign Address   State       PID/Program name
tcp        0      0 0.0.0.0:9527      0.0.0.0:*         LISTEN      1234/server
tcp        0      0 127.0.0.1:9527    127.0.0.1:53000   ESTABLISHED 1234/server
```

- `Proto`：协议类型，如 `tcp`、`udp`。
- `Recv-Q`：接收队列中还没有被应用程序读取的数据量。
- `Send-Q`：发送队列中还没有被对端确认的数据量。
- `Local Address`：本地 IP 和端口。
- `Foreign Address`：对端 IP 和端口。
- `State`：TCP 状态，如 `LISTEN`、`ESTABLISHED`、`TIME_WAIT`。
- `PID/Program name`：占用该 socket 的进程。

## 常见 TCP 状态

- `LISTEN`：服务器正在监听端口，通常说明 [[linux网络编程/函数笔记/Socket/listen|listen]] 已经生效。
- `ESTABLISHED`：连接已经建立，可以收发数据。
- `TIME_WAIT`：主动关闭方等待 [[linux网络编程/概念词条/MSL|2MSL]] 后释放连接。
- `CLOSE_WAIT`：本端收到对端关闭请求，但应用层还没有关闭 socket，常见原因是程序没有及时 `close`。
- `SYN_SENT`：客户端已经发送 SYN，正在等待服务器响应。
- `SYN_RECV`：服务器已经收到 SYN 并回复 SYN+ACK，正在等待客户端 ACK。

更多状态可看 [[linux网络编程/概念词条/TCP状态转换图|TCP状态转换图]]。

## 典型排查场景

### 查看端口是否监听

```bash
netstat -tln | grep 9527
```

如果能看到 `0.0.0.0:9527` 或 `127.0.0.1:9527` 且状态是 `LISTEN`，说明服务端已经监听该端口。

### 查看端口被哪个进程占用

```bash
sudo netstat -tulpn | grep 9527
```

`-p` 可以显示 PID 和程序名。没有权限时可能看不到完整进程信息。

### 查看 TCP 连接状态

```bash
netstat -tan | grep 9527
```

可以观察客户端和服务端之间是否已经进入 `ESTABLISHED`，或者是否残留大量 `TIME_WAIT`、`CLOSE_WAIT`。

### 查看路由表

```bash
netstat -rn
```

这个用法用于观察默认网关和路由规则。现代 Linux 也常用 `ip route`。

## 和 socket 编程的关系

- 写服务器时，调用 [[linux网络编程/函数笔记/Socket/bind|bind]] 和 [[linux网络编程/函数笔记/Socket/listen|listen]] 后，可以用 `netstat -tln` 检查端口是否进入 `LISTEN`。
- 客户端 [[linux网络编程/函数笔记/Socket/connect|connect]] 成功后，可以用 `netstat -tan` 看到 `ESTABLISHED`。
- 服务器重启遇到 `Address already in use` 时，可以用 `netstat -tanp` 排查端口占用和 TCP 状态。

## 和 ss、lsof 的区别

- [[linux网络编程/指令查询/命令卡/ss|ss]]：现代 Linux 推荐使用，速度更快，信息更直接。
- [[linux网络编程/指令查询/命令卡/lsof|lsof]]：更偏“按文件/端口反查进程”，例如 `lsof -i :9527`。
- `netstat`：旧系统和旧课程中非常常见，理解它有助于看懂资料和报错排查。

## 易错点

- 新系统可能默认没有 `netstat`，需要安装 `net-tools` 包。
- `-n` 很重要，否则端口可能被显示成服务名，不利于初学阶段直接判断。
- `-p` 显示进程信息通常需要 root 权限。
- `LISTEN` 只表示服务器在监听，不代表客户端一定能连通，还要考虑防火墙、IP 绑定、网络可达性。

## 相关指令

- [[linux网络编程/指令查询/命令卡/ss|ss]]
- [[linux网络编程/指令查询/命令卡/lsof|lsof]]
- [[linux网络编程/指令查询/命令卡/ping|ping]]
- [[linux网络编程/指令查询/命令卡/nc|nc]]

## 相关概念

- [[linux网络编程/概念词条/TCP|TCP]]
- [[linux网络编程/概念词条/TCP状态转换图|TCP状态转换图]]
- [[linux网络编程/概念词条/TIME_WAIT|TIME_WAIT]]
- [[linux网络编程/概念词条/MSL|MSL]]
- [[linux网络编程/概念词条/端口复用|端口复用]]

## 相关课时

- [[linux网络编程/课时笔记/04 高并发服务器/03 端口复用|03 端口复用]]
- [[linux网络编程/课时笔记/03 TCP通信与通信案例/02 客户端与服务器通信流程|02 客户端与服务器通信流程]]

## 相关模块

- [[linux网络编程/04 高并发服务器|04 高并发服务器]]
- [[linux网络编程/03 TCP通信与通信案例|03 TCP通信与通信案例]]
