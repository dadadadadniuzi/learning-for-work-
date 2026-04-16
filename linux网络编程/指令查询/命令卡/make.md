---
title: make
tags:
  - linux
  - 网络编程
  - 指令查询
---
# make

> [!info] 功能
> 按照 Makefile 中的规则自动编译项目。07 章安装 [[linux网络编程/概念词条/Libevent|Libevent]] 时会用它编译源码。

## 基本格式

```bash
make [目标]
```

## 常见用法

```bash
make
make install
make clean
```

## 常见目标

- `make`：执行默认目标，通常是编译整个项目。
- `make install`：把编译好的库、头文件、工具安装到系统目录或指定前缀目录。
- `make clean`：清理编译产生的中间文件。

## 在本课程中的作用

- 解压 Libevent 源码后，通常先执行 `./configure` 生成 Makefile。
- 执行 `make` 编译 Libevent。
- 执行 `sudo make install` 安装库和头文件。

## 易错点

- `make` 依赖当前目录下的 Makefile；如果目录不对，会提示找不到目标或找不到 Makefile。
- 安装到系统目录通常需要管理员权限。
- 编译自己的 Libevent 程序时，链接阶段通常需要加 `-levent`。

## 相关课时

- [[linux网络编程/课时笔记/07 Libevent库/01 Libevent简介与安装|01 Libevent简介与安装]]

## 相关模块

- [[linux网络编程/07 Libevent库|07 Libevent库]]
