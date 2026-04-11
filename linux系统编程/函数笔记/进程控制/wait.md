---
title: wait
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/进程控制
---
# wait

> [!info] 功能
> 等待并回收任意子进程。

## 函数原型

- `[[linux系统编程/概念词条/pid_t|pid_t]] wait(int *wstatus);`

## 依赖头文件

- `#include <sys/types.h>`
- `#include <sys/wait.h>`
- `#include <unistd.h>`

## 输入参数

- `wstatus`：输出子进程的结束状态。传 `NULL` 表示不关心状态，只回收子进程。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功返回被回收子进程的 `pid`。
- 没有可回收子进程时会阻塞等待。
- 出错返回 `-1`，并设置 `[[linux系统编程/概念词条/errno|errno]]`。

## 知识点补充

- `wait` 的作用是阻塞父进程，直到某个子进程结束并被回收。
- `wstatus` 里保存的是内核编码后的状态，必须配合 [[linux系统编程/概念词条/wait状态宏|WIFEXITED]]、[[linux系统编程/概念词条/wait状态宏|WEXITSTATUS]]、[[linux系统编程/概念词条/wait状态宏|WIFSIGNALED]]、[[linux系统编程/概念词条/wait状态宏|WTERMSIG]] 等宏解析。
- 它常用于回收僵尸子进程，避免子进程退出后只留下进程表项。
- 如果想精确等待指定子进程，或者不想阻塞等待，通常改用 `[[linux系统编程/函数笔记/进程控制/waitpid.md|waitpid]]`。

## 常见用法

- 父进程回收任意一个子进程。
- 配合 `[[linux系统编程/函数笔记/进程控制/fork.md|fork]]` 管理多个子进程。

## 易错点

- `wstatus` 必须传地址，例如 `int status; wait(&status);`。
- 状态值不要直接拿来和退出码比较，要先用宏判断。

## 相关概念

- [[linux系统编程/概念词条/pid_t|pid_t]]
- [[linux系统编程/概念词条/PCB|PCB]]
- [[linux系统编程/概念词条/wait状态宏|wait状态宏]]
- [[linux系统编程/概念词条/SIGCHLD|SIGCHLD]]

## 相关课时

- [[linux系统编程/课时笔记/05 进程控制与回收/01 wait与waitpid|01 wait与waitpid]]

## 相关模块

- [[linux系统编程/05 进程控制与回收|05 进程控制与回收]]
