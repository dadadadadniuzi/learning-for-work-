---
title: tar
tags:
  - linux
  - 系统编程
  - 指令查询
  - 命令卡片
---
# tar

> [!info] 功能
> 打包和归档文件。

## 语法

- `tar -cvf a.tar dir`
- `tar -xvf a.tar`
- `tar -zcvf a.tar.gz dir`
- `tar -jcvf a.tar.bz2 dir`

## 输入参数

- `c`：创建包。
- `x`：解包。
- `v`：显示过程。
- `f`：指定文件名。
- `z`：配合 gzip。
- `j`：配合 bzip2。

## 输出

- 生成归档文件，或解开归档文件。

## 常见用法

- 备份目录。
- 打包并压缩。

## 易错点

- `tar` 是“打包”的核心，压缩只是附加能力。

## 相关课时

- [[linux系统编程/课时笔记/01 Linux基础与开发环境/02 文件与目录操作命令]]

