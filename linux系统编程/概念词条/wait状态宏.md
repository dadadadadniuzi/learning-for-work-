---
title: wait状态宏
tags:
  - linux
  - 系统编程
  - 概念词条
  - 进程
---
# wait状态宏

## 它是什么
- `wait` 和 `waitpid` 会把子进程状态编码到一个整数里。
- 这个整数不是直接可读的退出码，需要用一组宏来解析。

## 常用宏
- `WIFEXITED(status)`：判断子进程是否正常退出。
- `WEXITSTATUS(status)`：获取子进程 `exit` 返回值。
- `WIFSIGNALED(status)`：判断子进程是否被信号终止。
- `WTERMSIG(status)`：获取导致子进程终止的信号编号。
- `WIFSTOPPED(status)`：判断子进程是否暂停。
- `WSTOPSIG(status)`：获取导致子进程暂停的信号编号。
- `WCOREDUMP(status)`：判断是否产生 core 文件，部分系统可用。

## 怎么理解
- `status` 只是内核编码后的状态包。
- 先判断类别，再取具体值，是最稳妥的使用顺序。
- 不要把 `status` 直接当成退出码。

## 相关入口
- [[linux系统编程/函数笔记/进程控制/wait|wait]]
- [[linux系统编程/函数笔记/进程控制/waitpid|waitpid]]
- [[linux系统编程/课时笔记/05 进程控制与回收/01 wait与waitpid|01 wait与waitpid]]
