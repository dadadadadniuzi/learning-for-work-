---
title: execvp
tags:
  - linux
  - 系统编程
  - 函数卡片
  - 系统编程/进程控制
---
# execvp

> [!info] 功能
> 用参数数组执行新程序，并搜索 PATH。

## 函数原型

- `int execvp(const char *file, char *const argv[]);`

## 依赖头文件

- `#include <unistd.h>`

## 输入参数

- `file`：可执行文件名或路径。
- `argv`：参数数组，最后必须以 `NULL` 结束。

## 输出参数

- 无直接输出参数。

## 返回值

- 成功不返回。
- 失败返回 `-1`，并设置 `[[linux系统编程/概念词条/errno|errno]]`。

## 知识点补充

- `execvp` 同时具备“数组参数”和“PATH 搜索”两个特征。
- 它可以理解成 `execv` 的 PATH 搜索版本。
- 成功后旧进程映像被替换，原来的代码不会继续执行。

## 常见用法

- 在需要搜索 `PATH` 且参数较多时使用。

## 易错点

- `argv` 不能少 `NULL` 结尾。

## 相关概念

- [[linux系统编程/课时笔记/05 进程控制与回收/02 exec函数族|exec函数族]]
- [[linux系统编程/课时笔记/05 进程控制与回收/03 fork-exec进程模型|fork-exec进程模型]]

## 相关课时

- [[linux系统编程/课时笔记/05 进程控制与回收/02 exec函数族|02 exec函数族]]

## 相关模块

- [[linux系统编程/05 进程控制与回收|05 进程控制与回收]]
