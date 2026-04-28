---
title: fcntl
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/文件IO
---
# fcntl

> [!info] 功能
> 执行[[linux系统编程/概念词条/文件描述符|文件描述符]]控制操作。

## 函数原型

- int fcntl(int fd, int cmd, ...);

## 依赖头文件

- `#include <unistd.h>`
- `#include <fcntl.h>`
## 输入参数

- fd：目标[[linux系统编程/概念词条/文件描述符|文件描述符]]。
- cmd：控制命令，如 [[linux系统编程/概念词条/fcntl命令|F_GETFL]]、[[linux系统编程/概念词条/fcntl命令|F_SETFL]]、[[linux系统编程/概念词条/fcntl命令|F_DUPFD]] 等。
- 第三个参数：取决于 cmd，可能是标志位、起始 fd 或额外参数。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回结果值，具体含义取决于 cmd。
- 失败返回 -1，并设置 [[linux系统编程/概念词条/errno|errno]]。

## 知识点补充

- `fcntl` 是[[linux系统编程/概念词条/文件描述符|文件描述符]]控制的统一入口。
- 它既能做复制，也能改属性，还能设置非阻塞等标志。
- 很多高级 I/O 行为都要通过它来完成。

## 常见用法

- 设置非阻塞、复制 fd、读取/修改 fd 标志。

## 网络编程里最常见的用法

在网络编程中，`fcntl` 最常见的场景之一就是把 socket 设置成非阻塞：

```c
int flags = fcntl(fd, F_GETFL, 0);
fcntl(fd, F_SETFL, flags | O_NONBLOCK);
```

这里的思路是：

- 先用 `F_GETFL` 取出原有标志。
- 再用 `F_SETFL` 把 [[linux系统编程/概念词条/open标志位|O_NONBLOCK]] 按位或回去。

这样不会把原来已有的状态标志直接覆盖掉。

相关网络知识：

- [[linux网络编程/概念词条/非阻塞I O|非阻塞I/O]]
- [[linux网络编程/概念词条/O_NONBLOCK|O_NONBLOCK]]
- [[linux网络编程/概念词条/EAGAIN与EWOULDBLOCK|EAGAIN与EWOULDBLOCK]]

## 易错点

- `cmd` 不同，第三个参数的含义完全不同。
- 设置非阻塞时不要直接写 `fcntl(fd, F_SETFL, O_NONBLOCK)`，否则可能把原来的状态标志覆盖掉。
- 在网络编程里看到 `read/recv/send/accept` 返回 `-1` 时，要结合 `errno` 判断是不是非阻塞下的正常现象。

## 相关概念

- [[linux系统编程/概念词条/fcntl命令|fcntl命令]]
- [[linux系统编程/概念词条/open标志位|open标志位]]
- [[linux系统编程/概念词条/文件描述符|文件描述符]]
- [[linux系统编程/概念词条/errno|errno]]
- [[linux系统编程/概念词条/perror|perror]]

## 相关课时

- [[linux系统编程/课时笔记/02 文件IO/05 dup-dup2-fcntl与重定向|05 dup-dup2-fcntl与重定向]]

## 相关模块

- [[linux系统编程/02 文件IO|02 文件IO]]


