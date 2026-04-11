---
title: ln
tags:
  - linux
  - 系统编程
  - 指令查询
  - 命令卡片
---
# ln

> [!info] 功能
> 创建链接。

## 语法

- `ln target linkname`
- `ln -s target linkname`

## 输入参数

- `target`：目标文件或目录。
- `linkname`：链接名。
- `-s`：创建软链接。

## 输出

- 生成硬链接或软链接。

## 常见用法

- 创建快捷访问入口。
- 给同一个 inode 增加别名。

## 易错点

- 硬链接和软链接的行为不同，尤其是跨文件系统和目录目标。

## 相关课时

- [[linux系统编程/课时笔记/03 目录与文件系统/03 unlink-rename与链接]]

