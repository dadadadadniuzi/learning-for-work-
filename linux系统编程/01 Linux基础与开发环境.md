---
title: Linux基础与开发环境
tags:
  - linux
  - 系统编程
  - 模块复习
  - 系统编程/基础
---
# Linux基础与开发环境

## 本章目标

- 熟悉 Linux 目录结构、路径、常用命令
- 理解 shell、vim、gcc、gdb、Makefile 在开发流程中的位置
- 能把“写代码 -> 编译 -> 调试 -> 构建”串成一条线
- 认识最常用的标准库基础函数，比如 [[linux系统编程/概念词条/memcpy|memcpy]]、[[linux系统编程/概念词条/memmove|memmove]]、[[linux系统编程/概念词条/memset|memset]]
- 认识项目代码里常见的基础转换函数，例如 [[linux系统编程/函数笔记/标准库/atoi|atoi]]
- 认识命令行参数解析里常见的函数，例如 [[linux系统编程/函数笔记/标准库/getopt|getopt]]

## 复习提纲

- Linux 常见目录与路径
- 文件和目录命令
- vim 三种模式
- gcc 四步编译
- 动态链接与静态链接
- gdb 基础命令
- Makefile 的目标、依赖与规则

## 课时导航

### 01 Linux基础与开发环境

- [[linux系统编程/课时笔记/01 Linux基础与开发环境/01 Linux目录、路径与常用命令]]
- [[linux系统编程/课时笔记/01 Linux基础与开发环境/02 文件与目录操作命令]]
- [[linux系统编程/课时笔记/01 Linux基础与开发环境/03 vim基础操作]]
- [[linux系统编程/课时笔记/01 Linux基础与开发环境/04 GCC编译流程与链接]]
- [[linux系统编程/课时笔记/01 Linux基础与开发环境/05 GDB与Makefile]]

## 命令直达

- [[pwd]] [[cd]] [[ls]] [[mkdir]] [[rmdir]] [[touch]] [[cp]] [[mv]] [[rm]]
- [[cat]] [[more]] [[less]] [[head]] [[tail]] [[find]] [[grep]] [[xargs]] [[awk]]
- [[chmod]] [[chown]] [[chgrp]] [[su]] [[ln]] [[tar]] [[gzip]] [[bzip2]] [[zip]] [[unzip]]
- [[which]] [[man]] [[history]] [[alias]] [[date]] [[file]] [[apt-get]] [[umask]]
- [[linux系统编程/概念词条/memcpy]] [[linux系统编程/概念词条/memmove]] [[linux系统编程/概念词条/memset]] [[linux系统编程/概念词条/memcmp]]

## 细节补充

- `pwd`、`cd`、`ls`、`find`、`grep` 是后面所有章节的底层工具。
- `umask` 要和 `chmod` 区分：一个管“新建时默认权限”，一个管“已有文件权限”。
- `gcc -E/-S/-c` 对应预处理、编译、汇编，最后链接才生成可执行文件。
- `gdb` 的重点不是背命令，而是学会定位程序到底在哪一步出错。
- `memcpy`、`memmove`、`memset` 是 C 语言里最常用的一组内存操作函数。
- `memcpy` 和 `memmove` 要区分重叠内存场景，`memset` 常用于初始化。
- `atoi` 常用于把命令行参数或配置字符串快速转换成整数。
- `getopt` 常用于解析 `-p 8080`、`-f conf.txt` 这类命令行选项。
