---
title: MySQL 深入
aliases:
  - MySQL日志
  - MySQL执行引擎
tags:
  - 八股
  - 数据库
  - MySQL
  - interview
  - database/mysql
status: active
created: 2026-06-09
---

# MySQL 深入

返回：[[数据库导航]]

相关：[[02 索引]] | [[03 进阶机制]]

> [!abstract]
> 这页整理 MySQL 执行引擎、日志、安全问题等高频题。

## 第一轮学习顺序

1. [[#MySQL 执行引擎是什么]]
2. [[#redo log、undo log、binlog 有什么区别]]
3. [[#SQL 注入是什么，怎么防范]]
4. [[#MySQL 常见存储引擎有哪些]]

## MySQL 执行引擎是什么

### 简要回答

MySQL 可以分为 Server 层和存储引擎层。Server 层负责 SQL 解析、优化和执行，存储引擎负责具体数据读写。

### 面试表达

> MySQL 的执行器不会直接操作磁盘数据，而是调用存储引擎接口。InnoDB 是最常用的存储引擎，支持事务、行锁和崩溃恢复。

## MySQL 常见存储引擎有哪些

### 简要回答

常见存储引擎有 InnoDB、MyISAM、Memory。

### 对比

| 存储引擎 | 特点 |
|---|---|
| InnoDB | 支持事务、行锁、外键、崩溃恢复，MySQL 默认常用 |
| MyISAM | 不支持事务，表锁，读性能较好，早期常用 |
| Memory | 数据存在内存中，速度快，但重启后数据丢失 |

### 面试表达

> 现在面试里 MySQL 存储引擎基本重点说 InnoDB。它支持事务、MVCC、行锁、聚簇索引和崩溃恢复，是业务系统最常用选择。

## redo log、undo log、binlog 有什么区别

### 简要回答

- redo log：重做日志，用于崩溃恢复，保证持久性。
- undo log：回滚日志，用于事务回滚和 MVCC。
- binlog：二进制日志，用于主从复制和数据恢复。

### 面试表达

> redo log 偏 InnoDB 存储引擎，用来保证崩溃后已提交事务能恢复；undo log 用来回滚和构造历史版本；binlog 是 MySQL Server 层日志，常用于复制和恢复。

### 详细回答

- redo log 是物理日志，记录“某个页做了什么修改”，用于宕机恢复。
- undo log 是逻辑日志，记录回滚所需的信息，也支持 MVCC 版本链。
- binlog 是 Server 层日志，记录 SQL 或行变更，用于主从复制和时间点恢复。

### 两阶段提交

MySQL 为了保证 redo log 和 binlog 一致，会使用两阶段提交：

```text
# 第一步：InnoDB 先把 redo log 写成 prepare 状态
# 含义：存储引擎已经准备好提交，但还没最终提交
redo log prepare -> 写 binlog -> redo log commit

# 第二步：MySQL Server 层写 binlog
# 含义：这次修改已经可以用于主从复制和基于 binlog 的恢复

# 第三步：InnoDB 把 redo log 改成 commit 状态
# 含义：事务最终提交完成
```

这样可以避免崩溃后 redo log 和 binlog 状态不一致。

### 为什么要两阶段提交

| 如果只写一半就崩溃 | 可能问题 |
|---|---|
| redo log 写了，binlog 没写 | 主库能恢复这次修改，但从库收不到 binlog，主从不一致 |
| binlog 写了，redo log 没提交 | 从库可能执行了这次修改，但主库崩溃恢复后没有这次修改 |

两阶段提交把 redo log 分成 `prepare` 和 `commit` 两个状态，崩溃恢复时可以结合 binlog 判断事务到底该提交还是回滚。

### 面试表达

> redo log 保证 InnoDB 自己的崩溃恢复，binlog 保证复制和归档恢复。两阶段提交的目的，就是让这两份日志对同一个事务的记录保持一致。

## SQL 注入是什么，怎么防范

### 简要回答

SQL 注入是攻击者把恶意 SQL 片段拼进用户输入，改变原 SQL 语义。

### 防范

- 使用预编译语句。
- 参数绑定。
- 不直接拼接用户输入。
- 最小权限账号。
- 输入校验。

相关：[[../../求职笔记/真实面试问题#34. MySQL 常见安全漏洞有哪些？|MySQL 安全漏洞]]

## 资料来源

- [MySQL 的执行引擎有哪些](https://notes.kamacoder.com/base/mysql-storage-engines.html)
- [MySQL 日志文件有哪几种](https://notes.kamacoder.com/base/mysql-log-files.html)
