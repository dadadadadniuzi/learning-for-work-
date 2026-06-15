---
title: Redis 高频问题
aliases:
  - Redis八股
tags:
  - 八股
  - 数据库
  - Redis
  - interview
  - database/redis
status: active
created: 2026-06-09
---

# Redis 高频问题

返回：[[数据库导航]]

相关：[[../Redis/redis]]

> [!abstract]
> 这页整理 Redis 数据结构、淘汰策略、过期策略、持久化、缓存异常、分布式锁等高频问题。

## 第一轮学习顺序

1. [[#Redis 常见数据结构有哪些]]
2. [[#Redis 过期策略有哪些]]
3. [[#Redis 淘汰策略有哪些]]
4. [[#RDB 和 AOF 有什么区别]]
5. [[#缓存雪崩、穿透、击穿有什么区别]]
6. [[#Redis 分布式锁怎么实现]]
7. [[#布隆过滤器是什么]]
8. [[#Redis 主从同步怎么实现]]

## Redis 常见数据结构有哪些

### 简要回答

常见有 String、Hash、List、Set、ZSet，实际底层会根据数据规模和编码策略选择不同实现。

### 使用场景

- String：缓存、计数器。
- Hash：对象属性。
- List：消息队列、时间线。
- Set：去重、集合运算。
- ZSet：排行榜、延迟队列。

### 常见命令

| 命令 | 作用 | 常见用途 |
|---|---|---|
| `SET` / `GET` | 写入 / 读取字符串 | 缓存、计数、简单状态 |
| `HSET` | 写入 Hash 字段 | 保存对象属性 |
| `LPUSH` | 从列表左侧插入 | 简单队列、任务列表 |
| `SADD` | 向集合加入元素 | 去重、标签、关注关系 |
| `ZADD` | 向有序集合加入元素和分数 | 排行榜、延迟任务 |

```text
# String：设置 key=name，value=ava
SET name ava

# String：读取 key=name 的值
GET name

# Hash：给 user:1 这个对象写入 name 和 age 两个字段
HSET user:1 name ava age 20

# List：把 task1 从左侧压入 queue 列表
LPUSH queue task1

# Set：把 cpp、linux 加入 tags 集合；集合会自动去重
SADD tags cpp linux

# ZSet：把 user1 加入 rank，有序分数是 100
# 排行榜通常按 score 排序
ZADD rank 100 user1
```

## Redis 过期策略有哪些

### 简要回答

Redis 使用惰性删除和定期删除结合。

### 详细回答

- 惰性删除：访问 key 时发现过期再删。
- 定期删除：周期性抽样检查并删除过期 key。

### 为什么不用每个 key 一个定时器

如果每个 key 都单独维护定时器，海量 key 会带来很大的 CPU 和调度开销。Redis 追求高性能，所以采用惰性删除 + 定期抽样删除。

### 代码示例

| 命令 | 作用 | 记忆点 |
|---|---|---|
| `EX seconds` | 设置秒级过期时间 | 写入时直接带 TTL |
| `TTL key` | 查看剩余过期秒数 | `-1` 表示永不过期，`-2` 表示 key 不存在 |
| `EXPIRE key seconds` | 给已有 key 设置过期时间 | 常用于续期或补 TTL |
| `PERSIST key` | 移除过期时间 | 把临时 key 变成永久 key |
| `KEEPTTL` | 修改值但保留原 TTL | Redis 6.0+ 支持 |

```text
# 设置登录 token，60 秒后过期
SET login:token:1001 abcdef EX 60

# 查看 token 还剩多少秒过期
TTL login:token:1001

# 把 token 过期时间改成 120 秒
EXPIRE login:token:1001 120

# 删除过期时间，让 token 变成不过期
PERSIST login:token:1001

# 修改 token 的值，但保留原来的过期时间
SET login:token:1001 xyz KEEPTTL
```

## Redis 淘汰策略有哪些

### 简要回答

内存不够时，Redis 会按配置选择淘汰策略，比如 LRU、LFU、随机淘汰、淘汰即将过期 key 等。

### 常见策略

| 策略 | 含义 | 好记法 |
|---|---|---|
| `allkeys-lru` | 所有 key 中淘汰最近最少使用的 | 全体 key 里删“最久没用” |
| `volatile-lru` | 只在设置了过期时间的 key 中按 LRU 淘汰 | 临时 key 里删“最久没用” |
| `allkeys-lfu` | 所有 key 中淘汰访问频率最低的 | 全体 key 里删“最少被用” |
| `volatile-lfu` | 只在设置了过期时间的 key 中按 LFU 淘汰 | 临时 key 里删“最少被用” |
| `allkeys-random` | 所有 key 中随机淘汰 | 全体 key 随机删 |
| `volatile-random` | 只在设置了过期时间的 key 中随机淘汰 | 临时 key 随机删 |
| `volatile-ttl` | 只在设置了过期时间的 key 中，优先淘汰快过期的 | 谁快过期先删谁 |
| `noeviction` | 不淘汰，写入时报错 | 内存满了拒绝写 |

### 面试表达

> 过期策略解决 key 到期后怎么删，淘汰策略解决内存满了以后删谁。两者不是一回事。

## RDB 和 AOF 有什么区别

### 简要回答

RDB 是快照持久化，AOF 是追加写命令日志。

### 对比表

| 对比点 | RDB | AOF |
|---|---|---|
| 形式 | 数据快照 | 命令日志 |
| 恢复速度 | 快 | 相对慢 |
| 数据完整性 | 可能丢最近数据 | 通常更好 |
| 文件大小 | 较小 | 可能较大 |

### RDB

RDB 会在某个时间点把 Redis 内存数据生成快照，默认文件通常是 `dump.rdb`。常见触发方式有配置规则、`SAVE`、`BGSAVE`。

- `SAVE`：同步保存，会阻塞主线程。
- `BGSAVE`：后台 fork 子进程生成快照，生产更常见。

### AOF

AOF 会把写命令追加到 AOF 文件中，重启时重放命令恢复数据。

常见刷盘策略：

- `always`：每次写都刷盘，安全但慢。
- `everysec`：每秒刷盘一次，生产常见。
- `no`：交给操作系统决定，性能好但风险更高。

如果 RDB 和 AOF 同时开启，Redis 重启时通常优先使用 AOF，因为它一般更完整。

## 缓存雪崩、穿透、击穿有什么区别

### 简要回答

- 雪崩：大量 key 同时失效，请求打到数据库。
- 穿透：查询不存在的数据，缓存和数据库都没有。
- 击穿：热点 key 失效，大量请求同时打到数据库。

### 解决思路

- 雪崩：过期时间加随机值、多级缓存、限流。
- 穿透：缓存空值、布隆过滤器。
- 击穿：互斥锁、热点 key 永不过期、提前刷新。

### 对比表

| 问题 | 核心特征 | 常见方案 |
|---|---|---|
| 缓存穿透 | 查不存在的数据 | 参数校验、缓存空值、布隆过滤器 |
| 缓存击穿 | 热点 key 过期 | 互斥锁、逻辑过期、热点预热 |
| 缓存雪崩 | 大量 key 失效或 Redis 不可用 | TTL 随机、高可用、多级缓存、限流降级 |

### 面试表达

> 穿透重点是挡住不存在的数据，击穿重点是保护热点 key 回源，雪崩重点是系统级兜底。

## Redis 分布式锁怎么实现

### 简要回答

常用 `SET lockKey requestId NX PX 30000`，保证加锁和设置过期时间是一个原子操作。

### 注意点

- value 要唯一，释放锁时校验 value。
- 解锁要用 Lua 脚本保证原子性。
- 过期时间要合理，避免业务没执行完锁过期。

### 加锁命令

```text
# lock_key：锁的名字，通常按业务资源命名，比如 lock:order:1001
# requestId：锁持有者的唯一标识，常用 UUID，防止误删别人的锁
# NX：只有 key 不存在时才设置成功，相当于“抢锁”
# PX 30000：设置 30000 毫秒过期时间，防止进程宕机后锁永远不释放
SET lock_key requestId NX PX 30000
```

含义：

- `NX`：key 不存在时才设置成功。
- `PX` / `EX`：设置过期时间，防止死锁。
- `requestId`：唯一标识锁的持有者，释放锁时防误删。

### 释放锁 Lua 脚本

```lua
-- KEYS[1] 是锁 key，例如 lock:order:1001
-- ARGV[1] 是当前线程/请求持有的 requestId
if redis.call("GET", KEYS[1]) == ARGV[1] then
    -- 只有锁的 value 和自己的 requestId 一致，才删除锁
    return redis.call("DEL", KEYS[1])
else
    -- value 不一致说明锁不是自己的，不能删
    return 0
end
```

### 易错点

- 不要把 `SETNX` 和 `EXPIRE` 分两步执行，否则中间宕机可能形成死锁。
- 锁 TTL 太短会导致业务没执行完锁就过期。
- 获取锁失败后不要疯狂自旋，要退避重试或快速失败。

## 布隆过滤器是什么

### 简要回答

布隆过滤器是一种高空间效率的概率型数据结构，用来判断元素“可能存在”或“一定不存在”。

### 工作原理

底层是位数组 + 多个哈希函数：

1. 插入元素时，用多个哈希函数算出多个位置，把这些 bit 置为 1。
2. 查询元素时，如果任意一个位置是 0，说明一定不存在。
3. 如果所有位置都是 1，说明可能存在。

### 特点

- 优点：内存占用小，查询快，适合海量 key。
- 缺点：有误判率；普通布隆过滤器不支持直接删除。

### Redis 场景

布隆过滤器常用于防缓存穿透。请求先过布隆过滤器，如果判断一定不存在，就直接拦截，不再查 Redis 和数据库。

常见实现：

- RedisBloom 模块：`BF.ADD`、`BF.EXISTS`
- 基于 Bitmap 自己实现

## Redis 主从同步怎么实现

### 简要回答

从节点连接主节点后发送 `PSYNC`，主节点根据复制 ID 和偏移量决定全量同步还是增量同步。

### 全量同步

首次复制、从节点刚上线、增量数据追不上时会走全量同步：

1. 从节点发送 `PSYNC ? -1`。
2. 主节点执行 `BGSAVE` 生成 RDB。
3. 主节点把 RDB 发给从节点。
4. 从节点清空旧数据并加载 RDB。
5. 主节点把生成 RDB 期间的增量写命令补发给从节点。

### 增量同步

短暂断线重连时，从节点携带 `replid + offset` 请求部分重同步。如果主节点的复制积压缓冲区里还保留缺失数据，就直接补发；否则只能全量同步。

### 命令示例

| 命令 | 作用 | 使用时机 |
|---|---|---|
| `REPLICAOF host port` | 让当前节点成为指定主节点的从节点 | 配置主从复制 |
| `INFO replication` | 查看复制状态 | 排查主从角色、延迟、复制偏移量 |
| `PSYNC ? -1` | 请求全量同步 | 从节点第一次连主节点 |
| `PSYNC replid offset` | 请求增量同步 | 从节点断线重连 |

```text
# 把当前 Redis 节点配置成 192.168.1.10:6379 的从节点
REPLICAOF 192.168.1.10 6379

# 查看当前节点的复制信息，比如 role、master_host、offset
INFO replication

# 首次同步：不知道主节点复制 ID 和偏移量，请求全量同步
PSYNC ? -1

# 断线重连：带上主节点复制 ID 和自己同步到的偏移量，请求增量同步
PSYNC <master_replid> <offset>
```

### 面试表达

> Redis 主从同步是最终一致，不是强一致。主节点写成功，不代表从节点已经立刻同步完成。

## 资料来源

- [Redis 常见的数据结构有哪些](https://notes.kamacoder.com/base/redis-data-structures.html)
- [Redis 缓存满了之后的淘汰策略](https://notes.kamacoder.com/base/redis-cache-eviction.html)
- [Redis 的过期策略](https://notes.kamacoder.com/base/redis-expiration-policies.html)
- [Redis 的持久化机制 RDB 和 AOF](https://notes.kamacoder.com/base/redis-persistence-rdb-aof.html)
- [Redis 的缓存雪崩、缓存穿透、缓存击穿](https://notes.kamacoder.com/base/redis-cache-stampeding-penetration-avalanche.html)
- [Redis 的分布式锁怎么实现](https://notes.kamacoder.com/base/redis-distributed-lock.html)
- [Redis 的布隆过滤器是什么](https://notes.kamacoder.com/base/redis-bloom-filter.html)
- [Redis 的主从同步是怎么实现的](https://notes.kamacoder.com/base/redis-master-slave-synchronization.html)
