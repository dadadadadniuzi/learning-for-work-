---
title: waitpid
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/进程控制
---
# waitpid

> [!info] 功能
> 等待并回收指定子进程。

## 函数原型

- [[linux系统编程/概念词条/pid_t|pid_t]] waitpid([[linux系统编程/概念词条/pid_t|pid_t]] pid, int \\*wstatus, int options);

## 依赖头文件

- `#include <sys/types.h>`
- `#include <sys/wait.h>`
- `#include <unistd.h>`

## 输入参数

- `pid`：指定等待对象。`> 0` 表示等待某个具体子进程；`0` 表示等待同进程组子进程；`-1` 表示等待任意子进程；`< -1` 表示等待进程组号为 `abs(pid)` 的子进程。
- `wstatus`：输出子进程状态，可传 `NULL`。
- `options`：控制等待行为，常见是 [[linux系统编程/概念词条/waitpid选项|WNOHANG]] 等。

## 输出参数

- 无直接输出参数。

## 返回值

- `> 0`：返回被回收子进程的 `pid`。
- `0`：配合 [[linux系统编程/概念词条/waitpid选项|WNOHANG]] 使用时表示暂时没有子进程状态变化。
- `-1`：出错并设置 [[linux系统编程/概念词条/errno|errno]]。

## 知识点补充

- `waitpid` 比 `wait` 更灵活，可以指定要等哪个子进程。
- `options` 决定是否阻塞，[[linux系统编程/概念词条/waitpid选项|WNOHANG]] 可以让调用立即返回，适合事件循环或信号处理场景。
- 它配合 [[linux系统编程/概念词条/SIGCHLD|SIGCHLD]] 很常见，可以用来循环回收多个退出的子进程。
- `wstatus` 仍然需要配合 [[linux系统编程/概念词条/wait状态宏|WIFEXITED]]、[[linux系统编程/概念词条/wait状态宏|WEXITSTATUS]]、[[linux系统编程/概念词条/wait状态宏|WIFSIGNALED]]、[[linux系统编程/概念词条/wait状态宏|WTERMSIG]] 等宏读取真正含义。

## 常见用法

- 非阻塞回收子进程。
- 只回收指定 `pid` 的孩子。
- 在 [[linux系统编程/概念词条/SIGCHLD|SIGCHLD]] 处理逻辑里循环调用。

## 易错点

- `pid` 的特殊取值容易混淆，复习时一定记住 `>0`、`0`、`-1`、`< -1` 四种含义。
- [[linux系统编程/概念词条/waitpid选项|WNOHANG]] 和阻塞等待的差异要分清楚。

## 相关概念

- [[linux系统编程/概念词条/pid_t|pid_t]]
- [[linux系统编程/概念词条/PCB|PCB]]
- [[linux系统编程/概念词条/wait状态宏|wait状态宏]]
- [[linux系统编程/概念词条/waitpid选项|waitpid选项]]
- [[linux系统编程/概念词条/SIGCHLD|SIGCHLD]]

## 相关课时

- [[linux系统编程/课时笔记/05 进程控制与回收/01 wait与waitpid|01 wait与waitpid]]
- [[linux系统编程/课时笔记/06 信号/04 SIGCHLD与子进程回收|04 SIGCHLD与子进程回收]]

## 相关模块

- [[linux系统编程/05 进程控制与回收|05 进程控制与回收]]

