---
title: tar
tags:
  - linux
  - 网络编程
  - 指令查询
---
# tar

> [!info] 功能
> 打包、解包或压缩归档文件。07 章安装 [[linux网络编程/概念词条/Libevent|Libevent]] 时常用它解压源码包。

## 基本格式

```bash
tar [选项] 归档文件 [文件或目录]
```

## 常用选项

- `-c`：创建归档文件。
- `-x`：解开归档文件。
- `-v`：显示处理过程。
- `-f`：指定归档文件名，后面必须跟文件名。
- `-z`：通过 gzip 处理，常见于 `.tar.gz`、`.tgz`。
- `-j`：通过 bzip2 处理，常见于 `.tar.bz2`。
- `-J`：通过 xz 处理，常见于 `.tar.xz`。
- `-C`：解压到指定目录。

## 常见用法

```bash
tar -zxvf libevent-2.1.12-stable.tar.gz
tar -Jxvf archive.tar.xz -C /tmp
tar -zcvf backup.tar.gz ./src
```

## 在本课程中的作用

- 下载 Libevent 源码包后，用 `tar -zxvf` 解压。
- 解压后进入源码目录，再执行配置、编译和安装步骤。

## 易错点

- `-f` 后面要紧跟归档文件名。
- 解压 `.tar.gz` 用 `-z`，解压 `.tar.xz` 用 `-J`，压缩格式选错会失败。

## 相关课时

- [[linux网络编程/课时笔记/07 Libevent库/01 Libevent简介与安装|01 Libevent简介与安装]]

## 相关模块

- [[linux网络编程/07 Libevent库|07 Libevent库]]
