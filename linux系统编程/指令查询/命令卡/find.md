---
title: find
tags:
  - linux
  - 系统编程
  - 指令查询
  - 命令卡片
---
# find

> [!info] 功能
> 按条件递归查找文件。

## 语法

- `find path -name "*.c"`
- `find path -type f`
- `find path -exec cmd {} \;`

## 输入参数

- `path`：查找起点目录。
- `-name`：按文件名模式匹配。
- `-type`：按文件类型匹配。
- `-size`：按大小匹配。
- `-mtime` / `-ctime` / `-atime`：按时间匹配。
- `-maxdepth`：限制递归深度。
- `-exec`：把查到的每个结果交给命令处理。

## 输出

- 满足条件的路径。

## 常见用法

- 找文件。
- 找特定类型、特定大小、特定时间的文件。

## 易错点

- `find` 查的是“文件”，不是内容。

## 相关课时

- [[linux系统编程/课时笔记/01 Linux基础与开发环境/02 文件与目录操作命令]]

