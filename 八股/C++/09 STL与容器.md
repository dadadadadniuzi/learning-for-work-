---
title: C++ STL 与容器
aliases:
  - STL与容器
  - C++容器
tags:
  - 八股
  - cpp
  - interview
  - cpp/stl
status: active
created: 2026-05-07
---

# C++ STL 与容器

返回：[[C++导航]]

相关：[[04 模板与泛型]] | [[03 指针引用与数组]] | [[08 面向对象导航]]

> [!abstract] 使用方式
> 这页作为 STL 与容器八股主入口。后续补充 `vector`、`map`、`unordered_map`、迭代器失效等内容时，优先并入这里；只有某个主题明显膨胀时再拆成独立专题页。

## 第一轮学习顺序

1. [[#STL 容器了解哪些]]
2. [[#vector 底层原理和扩容过程]]
3. [[#push_back 和 emplace_back 的区别]]
4. [[#STL 中迭代器失效的场景]]
5. [[#map 和 unordered_map 的区别与实现原理]]
6. [[#unordered_map 的 rehash 机制]]
7. [[#map deque list 的底层实现原理]]
8. [[#STL 中 allocator 的作用]]

## STL 容器了解哪些

### 简要回答

STL 容器大致可以分为：

1. 顺序容器：`vector`、`deque`、`list`、`array`、`forward_list`
2. 关联容器：`set`、`map`、`multiset`、`multimap`
3. 无序关联容器：`unordered_set`、`unordered_map`、`unordered_multiset`、`unordered_multimap`
4. 容器适配器：`stack`、`queue`、`priority_queue`

### 面试表达

如果面试官问“你了解哪些 STL 容器”，比较稳的答法是先按分类答，再顺手说出典型底层：

- `vector` 底层是动态数组
- `list` 底层是双向链表
- `deque` 底层是分段连续空间
- `map/set` 底层通常是红黑树
- `unordered_map/unordered_set` 底层通常是哈希表

## STL 中 allocator 的作用

### 先记结论

`allocator` 是 STL 的内存分配器，用来把“对象构造析构”和“原始内存分配释放”分离开。

### 后续要点

- 为什么容器不直接写死 `new/delete`
- `allocate` / `deallocate`
- `construct` / `destroy`
- 二级空间配置器思想
- 自定义 allocator 的使用场景

## STL 中迭代器失效的场景

### 先记结论

不同容器的迭代器失效规则不同：

- `vector` 扩容后大量迭代器、引用、指针会失效
- `deque` 插入删除中间元素时失效规则复杂
- `list` 插入通常不导致其他迭代器失效，被删除元素自己的迭代器失效
- `map/set` 插入通常不使已有迭代器失效，删除只会让被删元素迭代器失效
- `unordered_map` rehash 后迭代器通常会失效

### 后续要点

- 为什么 `vector` 扩容会失效
- erase 返回值怎么安全继续遍历
- 容器迭代失效的面试陷阱

## map 和 unordered_map 的区别与实现原理

### 先记结论

- `map` 底层通常是红黑树，元素有序，查找/插入/删除复杂度通常是 `O(log n)`
- `unordered_map` 底层通常是哈希表，元素无序，平均查找/插入/删除复杂度通常是 `O(1)`

### 后续要点

- 有序 vs 无序
- 最坏复杂度
- 红黑树和哈希表的适用场景
- 为什么 `unordered_map` 平均快，但不一定总比 `map` 好

## map deque list 的底层实现原理

### 先记结论

- `map`：红黑树
- `deque`：分段连续空间 + 中控映射表
- `list`：双向链表

### 后续要点

- 为什么 `deque` 支持头尾高效插入
- 为什么 `list` 随机访问慢
- 为什么 `map` 天然有序

## unordered_map 的 rehash 机制

### 先记结论

`unordered_map` 当负载因子过高时，通常会触发 rehash：

- 分配更大的桶数组
- 重新计算每个元素的新桶位置
- 把元素迁移过去

### 后续要点

- `load_factor`
- `max_load_factor`
- 为什么 rehash 会让迭代器失效
- 为什么 rehash 成本高

## vector 底层原理和扩容过程

### 先记结论

`vector` 底层是连续内存动态数组，支持随机访问。空间不足时会重新申请更大内存，把旧元素搬过去，再释放旧空间。

### 后续要点

- `size` 和 `capacity`
- 扩容倍率
- 为什么扩容会导致迭代器失效
- 拷贝搬迁和移动搬迁
- `reserve` 和 `resize` 的区别

## push_back 和 emplace_back 的区别

### 先记结论

- `push_back` 是把一个已有对象压进去
- `emplace_back` 是把构造参数传进去，在容器尾部原地构造对象

### 后续要点

- 是否一定更快
- 完美转发
- 对基础类型和复杂对象的区别

## 第二轮预留区

- `vector` vs `list` vs `deque`
- `set` vs `unordered_set`
- `reserve` vs `resize`
- 红黑树为什么不是 AVL
- 哈希冲突解决方式
