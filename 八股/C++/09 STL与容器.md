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

### 简要回答

`allocator` 是 STL 的内存分配器，用来把“对象构造析构”和“原始内存分配释放”分离开。

它通过模板参数把容器和具体内存管理策略解耦，使容器既能使用默认分配器，也能在特殊场景下切换成自定义分配器，比如内存池、共享内存、调试分配器等。

### 详细回答

#### allocator 的核心职责

`allocator` 主要负责 4 件事：

1. `allocate()`
   分配一段**原始内存**，但这时候对象还没被构造出来。

2. `deallocate()`
   释放之前分配的原始内存。

3. `construct()`
   在已经分配好的原始内存上构造对象。

4. `destroy()`
   调用对象析构函数，但不负责释放这块原始内存。

可以把它理解成：内存和对象生命周期被拆成了两层管理。

### 为什么 STL 需要 allocator

因为容器不应该把自己的实现写死在 `new/delete` 上。

STL 的设计思路是：

- 容器负责数据结构逻辑。
- allocator 负责内存管理策略。

这样就实现了职责分离。

好处是：

- 容器更通用。
- 可以替换不同内存策略。
- 可以做性能优化、内存追踪、特殊内存区域分配。

### 代码示例

```cpp
#include <iostream>
#include <memory>
using namespace std;

void basic_allocator_usage() {
    allocator<int> alloc;

    int* ptr = alloc.allocate(5);

    for (int i = 0; i < 5; ++i) {
        alloc.construct(ptr + i, i * 10);
    }

    for (int i = 0; i < 5; ++i) {
        cout << ptr[i] << " ";
    }
    cout << endl;

    for (int i = 0; i < 5; ++i) {
        alloc.destroy(ptr + i);
    }

    alloc.deallocate(ptr, 5);
}
```

这个例子正好体现出 allocator 的两层动作：

- 先 `allocate` 原始内存
- 再 `construct` 对象
- 销毁时先 `destroy`
- 最后 `deallocate`

### 设计理念

#### 1. 分离关注点

容器只管元素组织方式，比如：

- `vector` 管连续空间
- `list` 管链表节点
- `map` 管树结构

allocator 只管底层内存怎么申请和释放。

#### 2. 策略模式

allocator 作为模板参数传给容器，相当于把“内存管理策略”注入进去。

```cpp
std::vector<int, std::allocator<int>> vec;
```

如果你愿意，也可以替换成自己的 allocator。

#### 3. 类型安全和 rebind

容器内部不一定只分配 `T`。

比如 `list<T>` 内部真正分配的可能是节点类型 `list_node<T>`。这时候 allocator 需要有能力从“给 `T` 分配内存”切换成“给节点类型分配内存”。

这类能力传统上靠 `rebind` 完成。

### 什么是 rebind 机制

`rebind` 是早期 allocator 设计中的一种模板技术，用来把一个“给 `T` 分配”的分配器，重新绑定成“给 `U` 分配”的分配器。

大致形式：

```cpp
template <class U>
struct rebind {
    typedef allocator<U> other;
};
```

这样容器内部就可以从：

- `allocator<T>`

转换成：

- `allocator<node_type>`

现代标准库实现里这部分已经更体系化了，但面试里知道“rebind 是为了给容器内部其他类型分配内存”就够了。

### 什么情况下会用自定义 allocator

1. 使用内存池优化性能时。
2. 使用特殊内存区域时，比如共享内存、GPU 内存、NUMA 特定区域。
3. 做内存使用监控和调试时。
4. 需要特定对齐要求时。
5. 想实现统一的对象分配策略时。

### 自定义 allocator 要注意什么

1. 异常安全。
2. 对齐要求必须正确处理。
3. 容器传播属性要符合预期，比如 `propagate_on_container_xxx`。
4. 如果在并发环境共享使用，还要考虑线程安全。

### 面试官可能追问

#### Q1：什么情况下应该使用自定义 allocator？

需要做性能优化、特殊内存区域分配、调试追踪、定制内存布局时，都会考虑自定义 allocator。普通业务代码大多数时候用默认 allocator 就够了。

#### Q2：自定义 allocator 需要注意哪些陷阱？

重点是异常安全、对齐、传播语义和线程安全。如果这些点没处理好，容器行为会很隐蔽地出错。

#### Q3：allocator 和直接 new/delete 的本质区别是什么？

`new/delete` 把“分配内存”和“构造析构对象”绑在了一起；allocator 把这两件事拆开，让容器可以更细粒度地控制对象生命周期和底层内存策略。

## STL 中迭代器失效的场景

### 简要回答

不同容器的迭代器失效规则不同：

- `vector` 扩容后大量迭代器、引用、指针会失效
- `deque` 插入删除中间元素时失效规则复杂
- `list` 插入通常不导致其他迭代器失效，被删除元素自己的迭代器失效
- `map/set` 插入通常不使已有迭代器失效，删除只会让被删元素迭代器失效
- `unordered_map` rehash 后迭代器通常会失效

继续使用已经失效的迭代器，属于未定义行为。

### 详细回答

#### 失效的根本原因

迭代器失效本质上是“迭代器原来指向的位置已经不再有效”。

常见原因有三类：

1. 内存重新分配。
   比如 `vector` 扩容后，旧空间整体被释放。

2. 元素位置移动。
   比如顺序容器中间插入或删除，后面的元素整体搬迁。

3. 数据结构重构。
   比如 `unordered_map` rehash，桶结构被整体重建。

### 按容器分类理解

#### 1. vector / string

- 插入可能触发扩容，一旦重新分配，原有迭代器、引用、指针通常全部失效。
- 删除元素后，被删元素及其后面的迭代器通常会失效。

#### 2. deque

- 头尾插入删除通常相对高效。
- 中间插入删除失效规则更复杂，很多实现下可能导致大量迭代器失效。

#### 3. list / forward_list

- 插入元素通常不会导致其他迭代器失效。
- 删除时，只有指向被删除元素的迭代器失效。

#### 4. map / set

- 插入一般不会导致已有迭代器失效。
- 删除时，只有指向被删元素的迭代器失效。

#### 5. unordered_*

- 删除时通常只有被删元素迭代器失效。
- 插入如果触发 rehash，通常会导致所有迭代器失效。

### vector 的典型失效场景

```cpp
#include <iostream>
#include <vector>
using namespace std;

void vector_invalidation_examples() {
    vector<int> vec = {1, 2, 3, 4, 5};

    auto it = vec.begin();
    cout << "Before: " << *it << endl;

    for (int i = 0; i < 100; ++i) {
        vec.push_back(i);
    }
    // it 已失效，不能再用

    vec = {1, 2, 3, 4, 5};
    auto it1 = vec.begin() + 1; // 指向 2
    auto it2 = vec.begin() + 3; // 指向 4

    vec.erase(vec.begin() + 2); // 删除 3
    cout << *it1 << endl;       // 仍安全
    // cout << *it2 << endl;    // 危险，it2 已失效
}
```

### 正确使用方式

#### vector 删除时用 erase 返回值续上

```cpp
std::vector<int> vec = {1, 2, 3, 4, 3, 5};

for (auto it = vec.begin(); it != vec.end(); ) {
    if (*it == 3) {
        it = vec.erase(it); // 返回下一个有效迭代器
    } else {
        ++it;
    }
}
```

#### list 删除相对更安全

```cpp
std::list<int> lst = {1, 2, 3, 4, 5};
auto it = lst.begin();
++it; // 指向 2

lst.erase(lst.begin()); // 删除 1
std::cout << *it << std::endl; // 仍然安全
```

#### map 删除时只影响被删元素

```cpp
std::map<int, std::string> mp = {
    {1, "one"}, {2, "two"}, {3, "three"}
};

auto it = mp.find(2);
if (it != mp.end()) {
    mp.erase(it);
}
```

### 速记表

| 容器 | 插入操作 | 删除操作 |
|---|---|---|
| `vector/string` | 可能全部失效（扩容） | 被删元素及之后通常失效 |
| `deque` | 可能大量失效 | 规则复杂，常要谨慎 |
| `list` | 其他迭代器通常不失效 | 只有被删元素失效 |
| `map/set` | 其他迭代器通常不失效 | 只有被删元素失效 |
| `unordered_*` | 触发 rehash 时可能全部失效 | 只有被删元素失效 |

### 调试与排查技巧

1. 用容器操作后，默认怀疑旧迭代器是否还有效。
2. 删除元素时优先使用 `erase` 返回值继续遍历。
3. 调试阶段可打开调试版本 STL，例如某些环境下使用 `_GLIBCXX_DEBUG` 检查迭代器有效性。

### 面试官可能追问

#### Q1：如何在多线程环境中处理迭代器失效？

多线程下不仅有迭代器失效问题，还有数据竞争问题。通常要：

- 用互斥锁保护容器读写。
- 避免一个线程遍历时另一个线程修改同一容器。
- 必要时使用线程安全容器或读写分离设计。

#### Q2：调试迭代器失效有哪些技巧？

重点是：

- 删除后立即接住 `erase` 返回值。
- 容器修改后不要继续相信旧迭代器。
- 在调试环境启用 STL 调试模式，能更早暴露非法迭代器使用。

#### Q3：为什么 vector 扩容会导致迭代器失效？

因为 `vector` 底层是连续内存。如果旧空间不够，容器会申请一块更大的新内存，把原元素搬过去，再释放旧空间。旧迭代器原来指向的地址已经不存在了，所以会失效。

## map 和 unordered_map 的区别与实现原理

### 简要回答

- `map` 底层通常是红黑树，元素有序，查找/插入/删除复杂度通常是 `O(log n)`
- `unordered_map` 底层通常是哈希表，元素无序，平均查找/插入/删除复杂度通常是 `O(1)`

### 详细回答

#### 核心区别

| 特性 | `map` | `unordered_map` |
|---|---|---|
| 底层结构 | 红黑树 | 哈希表 |
| 键值顺序 | 自动有序，按 key 排序 | 无序 |
| 查找复杂度 | `O(log n)` | 平均 `O(1)`，最坏 `O(n)` |
| 插入复杂度 | `O(log n)` | 平均 `O(1)`，最坏 `O(n)` |
| 删除复杂度 | `O(log n)` | 平均 `O(1)`，最坏 `O(n)` |
| 自定义规则 | 支持比较器 | 支持自定义哈希函数和相等判定 |
| 内存占用 | 相对稳定 | 通常更大，需要桶数组和预留空间 |

### map 的实现原理

`map` 底层通常是红黑树，也就是一种平衡二叉搜索树。

它的特点是：

- 元素天然有序。
- 查找、插入、删除都能稳定保持 `O(log n)`。
- 插入或删除时需要通过旋转和染色维持平衡。

这也是为什么：

- `map` 适合有序遍历。
- `map` 适合范围查询。
- `map` 的复杂度更稳定。

### unordered_map 的实现原理

`unordered_map` 底层通常是哈希表。

基本流程是：

1. 对 key 计算哈希值。
2. 根据哈希值定位桶号。
3. 到对应桶中查找元素。
4. 如果桶里已经有冲突元素，就在桶内部继续找。

冲突严重时，性能会下降。

### 哈希冲突怎么处理

当多个 key 计算出来落到同一个桶时，就发生哈希冲突。

STL 实现中通常采用链式地址法，也就是同一个桶里挂一个链式结构或节点序列来保存这些元素。

这意味着：

- 冲突越少，查找越快。
- 冲突越多，桶内查找越慢。
- 极端情况下会退化得很厉害。

### 为什么 unordered_map 最坏是 O(n)

因为如果所有元素都哈希到同一个桶里，那么这个桶内部就相当于退化成线性结构。

此时查找、插入、删除都要把这一整串元素扫一遍，时间复杂度就退化为 `O(n)`。

### 代码示例

```cpp
#include <iostream>
#include <map>
#include <unordered_map>
using namespace std;

int main() {
    map<string, int> ordered_map;
    unordered_map<string, int> hash_map;

    ordered_map["apple"] = 1;
    ordered_map["banana"] = 2;
    ordered_map["orange"] = 3;

    hash_map["apple"] = 1;
    hash_map["banana"] = 2;
    hash_map["orange"] = 3;

    cout << "map（有序）遍历结果：" << endl;
    for (auto& p : ordered_map) {
        cout << p.first << " => " << p.second << endl;
    }

    cout << "\nunordered_map（无序）遍历结果：" << endl;
    for (auto& p : hash_map) {
        cout << p.first << " => " << p.second << endl;
    }

    return 0;
}
```

面试里可以强调：

- `map` 遍历结果稳定有序。
- `unordered_map` 遍历顺序不稳定，不能依赖。

### 什么时候用 map

1. 需要按 key 自动排序。
2. 需要范围查询。
3. 需要复杂度更稳定。
4. key 不适合做哈希，或者自定义哈希成本高。

### 什么时候用 unordered_map

1. 只关心快速查找和插入。
2. 元素很多、查找频繁。
3. 不关心元素顺序。
4. key 天然适合做哈希。

### 多线程下的差异

无论 `map` 还是 `unordered_map`，标准容器本身都不是线程安全的，尤其不能直接并发写。

但从结构上理解：

- `map` 结构更稳定，很多场景下适合读多写少，配合 `shared_mutex` 做读写分离。
- `unordered_map` 平均查找快，但 rehash、桶冲突、并发写会让问题更复杂。

如果真要并发高强度使用哈希容器，通常需要：

- 外层加锁
- 分桶加锁
- 或直接用专门的并发容器实现

### 内存占用差异

`map` 每个节点通常会保存：

- 左右孩子指针
- 父节点指针
- 颜色位
- 键值对

`unordered_map` 通常除了节点本身外，还要额外维护桶数组，并预留空槽位降低冲突。

因此在元素量相同时，`unordered_map` 的内存占用通常往往比 `map` 更高，但换来了平均更快的查找速度。

### 面试官可能追问

#### Q1：unordered_map 发生哈希冲突会怎么处理？

通常会把冲突元素放进同一个桶中，在桶内部再做链式查找。冲突越多，查找性能越差。

#### Q2：为什么 unordered_map 最坏情况是 O(n)？

因为极端情况下所有元素都落进同一个桶，哈希表退化成线性结构，查找、插入、删除都要线性扫描。

#### Q3：map 和 unordered_map 的内存占用怎么比较？

`map` 节点结构稳定；`unordered_map` 需要额外桶数组和预留空间，所以通常 `unordered_map` 内存占用更高一些。

#### Q4：哈希表的负载因子是什么？扩容机制如何？

负载因子通常是：

```text
load_factor = 元素个数 / 桶个数
```

表示哈希表的拥挤程度。负载因子太高时，冲突增多，性能下降，所以容器通常会触发扩容，并把所有元素重新哈希到新的桶数组中。

相关延伸：

- [[#unordered_map 的 rehash 机制]]
- [[#STL 中迭代器失效的场景]]

## map deque list 的底层实现原理

### 简要回答

- `map`：红黑树
- `deque`：分段连续空间 + 中控映射表
- `list`：双向链表

### 详细回答

这一题在 C++ 面试里，重点是把三者的底层结构、时间复杂度和适用场景讲清楚。

注意：这里说的是 **C++ STL** 里的 `std::map`、`std::deque`、`std::list`，不要和 Java 里的 `HashMap`、`TreeMap`、`ArrayDeque`、`ArrayList` 混在一起讲。

### 1. map 的实现原理

`std::map` 底层通常是红黑树，也就是一种自平衡二叉搜索树。

它的特点是：

- 按 key 自动有序排列。
- 查找、插入、删除复杂度通常是 `O(log n)`。
- 插入删除时会通过旋转和染色维护平衡。

这也是为什么 `map`：

- 适合按顺序遍历。
- 适合范围查询。
- 复杂度更稳定。

```cpp
std::map<std::string, int> m;
m["apple"] = 5;
m["banana"] = 3;

for (const auto& p : m) {
    std::cout << p.first << ": " << p.second << "\n";
}
```

### 2. deque 的实现原理

`std::deque` 通常不是单一连续大数组，而是“分段连续空间”。

可以把它理解成：

- 外层有一个中控映射表。
- 中控表里记录多个固定大小的缓冲块。
- 真正元素存放在这些缓冲块中。

所以它具备两个特点：

- 头尾插入删除都高效。
- 支持随机访问，但访问时通常需要先定位块，再定位块内偏移。

这也是为什么 `deque` 很适合：

- 双端队列场景。
- 滑动窗口。
- 需要头尾都频繁操作，同时又希望保留随机访问能力的情况。

```cpp
std::deque<int> dq = {2, 3, 4};
dq.push_front(1);
dq.push_back(5);
std::cout << dq[2] << "\n";
```

### 3. list 的实现原理

`std::list` 底层通常是双向链表。

每个节点里通常有：

- 当前元素值
- 指向前一个节点的指针
- 指向后一个节点的指针

它的特点是：

- 任意位置插入删除高效，只需要改指针。
- 元素地址稳定性较好。
- 不支持随机访问，访问第 `n` 个元素必须从头或尾遍历过去。
- 每个元素额外内存开销较大。

```cpp
std::list<int> lst = {1, 2, 4};
auto it = lst.begin();
std::advance(it, 2);
lst.insert(it, 3);
```

### 对比总结

| 容器 | 底层实现 | 随机访问 | 中间插入删除 | 头尾操作 | 是否有序 |
|---|---|---|---|---|---|
| `map` | 红黑树 | 不支持 | `O(log n)` | 不强调 | 按 key 有序 |
| `deque` | 分段连续空间 | `O(1)` | 较慢 | 头尾都高效 | 保持插入顺序 |
| `list` | 双向链表 | 不支持 | 高效 | 头尾都高效 | 保持插入顺序 |

### 时间复杂度对比

下面这张表用 C++ STL 语境来记更合适：

| 操作 | `unordered_map` | `map` | `deque` | `list` |
|---|---|---|---|---|
| 插入 | 平均 `O(1)`，最坏 `O(n)` | `O(log n)` | 头尾通常 `O(1)`，中间较慢 | 已知位置插入 `O(1)` |
| 删除 | 平均 `O(1)`，最坏 `O(n)` | `O(log n)` | 头尾通常 `O(1)`，中间较慢 | 已知位置删除 `O(1)` |
| 查找 | 平均 `O(1)`，最坏 `O(n)` | `O(log n)` | `O(n)` | `O(n)` |
| 随机访问 | 不支持 | 不支持 | `O(1)` | 不支持，按下标访问是 `O(n)` |
| 头尾操作 | 不强调 | 不强调 | `O(1)` | `O(1)` |

### 说明

1. `unordered_map` 的平均复杂度快，但最坏会因为哈希冲突退化。
2. `map` 的复杂度不如 `unordered_map` 平均快，但更稳定，而且天然有序。
3. `deque` 的优势是头尾操作和随机访问兼得，但中间插删不占优。
4. `list` 的优势是已知迭代器位置时插删快，但查找和随机访问都弱。

### 适用场景

| 容器 | 适合场景 | 不适合场景 |
|---|---|---|
| `map` | 需要有序键值存储、范围查询、稳定 `O(log n)` | 只追求平均 `O(1)` 查找 |
| `deque` | 两端频繁插入删除，同时需要随机访问 | 中间频繁插入删除、特别依赖元素地址稳定 |
| `list` | 中间频繁插入删除、迭代器和元素地址稳定性要求高 | 需要快速查找或随机访问 |

### 代码示例

```cpp
#include <deque>
#include <iostream>
#include <list>
#include <map>
using namespace std;

int main() {
    map<string, int> m;
    m["apple"] = 5;
    m["banana"] = 3;
    for (const auto& p : m) {
        cout << p.first << ": " << p.second << "\n";
    }

    deque<int> dq = {2, 3, 4};
    dq.push_front(1);
    dq.push_back(5);
    cout << dq[2] << "\n";

    list<int> lst = {1, 2, 4};
    auto it = lst.begin();
    advance(it, 2);
    lst.insert(it, 3);

    return 0;
}
```

### 面试官可能追问

#### Q1：为什么 deque 的中间插入通常比 list 慢？

因为 `deque` 为了维持分段顺序结构，中间插入往往需要移动一部分元素，甚至跨多个缓冲块调整；而 `list` 只需要改相邻节点指针，不需要移动已有元素。

#### Q2：为什么 list 随机访问慢？

因为 `list` 底层是链表，没有像数组那样的下标地址计算能力。访问第 `n` 个元素只能一个节点一个节点走过去。

#### Q3：为什么 map 天然有序？

因为 `map` 底层是红黑树，节点按 key 的比较规则组织，所以中序遍历天然就是有序结果。

## unordered_map 的 rehash 机制

### 简要回答

`unordered_map` 当负载因子过高时，通常会触发 rehash：

- 分配更大的桶数组
- 重新计算每个元素的新桶位置
- 把元素迁移过去

负载因子本质上是：

```text
load_factor = size() / bucket_count()
```

也就是“元素个数 / 桶个数”，表示哈希表当前有多拥挤。

当负载因子超过最大负载因子时，`unordered_map` 通常会自动触发 `rehash`，用更大的桶数组重新组织所有元素，以保持平均 `O(1)` 的查找、插入性能。

### 详细回答

#### 1. 什么是负载因子

负载因子是衡量哈希表拥挤程度的指标。

公式是：

```text
负载因子 = size() / bucket_count()
```

其中：

- `size()`：当前容器里键值对数量
- `bucket_count()`：桶数组大小

负载因子越高，说明平均每个桶里元素越多，冲突通常越严重。

#### 2. 负载因子为什么重要

`unordered_map` 之所以平均快，是因为希望元素能比较均匀地分散到不同桶里。

如果负载因子太高：

- 每个桶平均元素变多
- 冲突加重
- 桶内查找成本上升

这会让原本平均 `O(1)` 的查找和插入，越来越接近线性扫描。

#### 3. 最大负载因子

`unordered_map` 还有一个概念叫最大负载因子：

- `max_load_factor()`

它表示容器能接受的最大拥挤程度。

默认值通常是 `1.0` 左右，但这是实现细节层面常见情况，标准库允许实现自行决定默认策略。

当插入新元素后，新的负载因子超过这个上限，容器通常就会自动触发 `rehash`。

#### 4. rehash 有哪些触发方式

有两类：

##### 自动触发

插入元素后，如果：

```text
size() / bucket_count() > max_load_factor()
```

容器通常会自动扩容并重新哈希。

##### 手动触发

程序员也可以主动调用：

- `rehash(n)`
- `reserve(m)`

它们的区别是：

###### `rehash(n)`

含义是：至少把桶数量调到 `n` 这么大。

它从“桶数量”角度出发。

###### `reserve(m)`

含义是：为至少 `m` 个元素预留空间，尽量避免后续插入过程中频繁 rehash。

它从“元素数量”角度出发，内部通常会根据当前最大负载因子换算出需要多少桶。

可以粗略理解成：

```text
rehash(ceil(m / max_load_factor()))
//ceil(x) = 不小于 x 的最小整数
```

面试里一般更推荐说：

`reserve` 语义更直观，因为我们平时更关心“我要放多少个元素”，而不是“我要几个桶”。

#### 5. rehash 具体过程

rehash 发生时，大致会做这些事：

1. 分配一个新的、更大的桶数组
2. 遍历旧桶数组里的所有元素
3. 对每个元素重新计算桶位置
4. 把元素移动到新的桶中
5. 释放旧桶数组

所以 rehash 本质上是一场“全表搬家”。

#### 6. rehash 的复杂度为什么是 O(n)

因为它必须遍历所有元素，并重新计算哈希归属桶的位置。

如果当前有 `n` 个元素，那么 rehash 的时间复杂度通常就是 `O(n)`。

这也是为什么：

- 平时查找和插入很快
- 但某一次扩容可能突然变慢

这类性能波动是哈希表典型特点。

#### 7. rehash 后什么会失效

最重要的结论：

- 迭代器通常会全部失效

因为桶结构被重建，原来的遍历路径已经不成立。

在很多常见节点式实现里：

- 元素对象本身的引用和指针通常仍然有效

但面试时最稳的说法是：

> rehash 后，迭代器一定要视为失效，不能继续使用。

#### 8. 什么时候适合用 unordered_map

适合：

1. 只关心快速查找和插入
2. 不要求元素有序
3. 键有较好的哈希函数
4. 可以接受偶尔 rehash 带来的性能波动

不适合：

1. 需要按 key 排序
2. 不能接受最坏情况退化
3. 对内存特别敏感
4. 对实时性要求极高，不希望出现偶发延迟峰值

### 面试官可能追问

#### Q1：rehash 的复杂度是多少？为什么？

`O(n)`。

因为要遍历所有元素，重新计算它们的新桶位置，并插入到新的桶数组中。

#### Q2：reserve 和 rehash 有什么区别？

- `reserve(n)`：从“元素数量”角度出发，保证大致容纳 `n` 个元素前不频繁 rehash。
- `rehash(n)`：从“桶数量”角度出发，直接要求桶数量至少为 `n`。

通常 `reserve` 更符合日常使用语义。

#### Q3：rehash 后，迭代器、引用、指针会失效吗？

最该记的是：

- 迭代器会失效

至于引用和指针，在常见节点式实现中通常仍然可用，但面试里不要把话说得太满，最稳的回答是优先强调“迭代器失效，不能继续用旧迭代器”。

#### Q4：除了负载因子，还有什么情况会触发 rehash？

手动调用 `rehash()` 或 `reserve()` 时也会触发。

### 面试短答

> `unordered_map` 的负载因子是元素数量和桶数量的比值，用来衡量哈希表有多拥挤。负载因子超过最大负载因子时，容器通常会触发 `rehash`：分配更大的桶数组，把所有元素重新计算桶位置后搬过去。这个过程复杂度是 `O(n)`，而且会让迭代器失效。日常如果预先知道大概元素数量，通常会优先用 `reserve` 提前预留空间，减少频繁 rehash。

## vector 底层原理和扩容过程

### 简要回答

`vector` 底层是连续内存动态数组，支持随机访问。空间不足时会重新申请更大内存，把旧元素搬过去，再释放旧空间。

简单说可以记成三件事：

1. 开头在哪
2. 当前用了多少
3. 总共还能装多少

当装满了还要继续插入，它就会：

- 找一块更大的新内存
- 把旧元素搬过去
- 释放旧内存
- 记录新的容量和边界

### 详细回答

#### 底层结构

`vector` 可以理解成一个“会自动长大的数组”。

它底层通常维护三段信息，很多实现里会体现成三个指针或等价状态：

1. `start`
   指向首元素位置。

2. `finish`
   指向当前最后一个有效元素的下一个位置，也就是逻辑结束位置。

3. `end_of_storage`
   指向整块已分配存储空间的末尾。

如果用更口语的说法：

- `start`：开头在哪
- `finish`：现在用了多少
- `end_of_storage`：总共能装多少

这也对应 STL 常见两个接口：

- `size()`：当前已经存了多少元素
- `capacity()`：当前一共预留了多少容量

#### 为什么 vector 支持随机访问

因为它的数据在内存中是连续存放的。

所以访问第 `i` 个元素时，只需要做地址偏移：

```text
起始地址 + i * sizeof(T)
```

这就是 `vector` 随机访问是 `O(1)` 的原因。

### 扩容机制

#### 什么时候触发扩容

当：

```text
size() == capacity()
```

时，再插入新元素就放不下了，这时通常会触发扩容。

#### 扩容过程

典型流程是：

1. 申请一块更大的新内存。
2. 把旧元素搬到新内存中。
3. 销毁旧内存中的对象。
4. 释放旧内存。
5. 更新内部指针或边界信息。

#### 扩容倍率

具体倍率由标准库实现决定，不是标准强制统一的。

很多实现里会按大约 2 倍增长，或者在不同阶段采用略有差异的增长策略。面试里比较稳的说法是：

> 通常按倍数增长，而不是每次只多一点空间。

### 为什么不按固定大小扩容

如果每次都只多固定一点，比如每次多 1 个位置，那么元素一多，就会频繁扩容、频繁搬家，总体成本很高。

而按倍数扩容，虽然某次扩容比较重，但平均摊到每次 `push_back` 上，尾插复杂度仍然可以看作均摊 `O(1)`。

### 时间复杂度

| 操作 | 复杂度 |
|---|---|
| 随机访问 | `O(1)` |
| 尾部插入 / 删除 | 均摊 `O(1)` |
| 中间插入 / 删除 | `O(n)` |

### 代码示例

```cpp
#include <iostream>
#include <vector>
using namespace std;

void printVectorInfo(const vector<int>& v) {
    cout << "Size: " << v.size()
         << ", Capacity: " << v.capacity();

    if (!v.empty()) {
        cout << ", Address: " << &v[0];
    }
    cout << endl;
}

int main() {
    vector<int> vec;

    for (int i = 0; i < 20; ++i) {
        vec.push_back(i);
        printVectorInfo(vec);
    }

    vector<int> vec2;
    vec2.reserve(100);
    cout << "\nAfter reserve:\n";
    printVectorInfo(vec2);

    return 0;
}
```

这个例子可以观察：

- `size()` 怎么增长
- `capacity()` 什么时候跳变
- 扩容后底层地址可能发生变化

### 为什么扩容会导致迭代器失效

因为扩容时会申请新的连续内存，然后把元素搬到新地方去。

原来的：

- 迭代器
- 指针
- 引用

如果指向的是旧内存，就都不能再继续用了。

这也是 `vector` 的一个高频坑。

### reserve 和 resize 的区别

#### `reserve(n)`

只扩容量，不改逻辑元素个数。

```cpp
vec.reserve(100);
```

这表示提前准备至少 100 个元素的空间，但 `size()` 不会变成 100。

#### `resize(n)`

会直接改变逻辑元素个数。

```cpp
vec.resize(100);
```

这不仅可能扩容，还会真的让容器里有 100 个元素。

面试里一句话：

- `reserve` 改容量
- `resize` 改大小

### vector 和 list 的主要区别

- `vector` 连续存储，随机访问快
- `list` 链表存储，中间插删快

所以：

- 频繁随机访问、主要尾插：优先 `vector`
- 中间频繁插删、要求节点地址稳定：更偏 `list`

### 适用场景

适合：

1. 需要频繁随机访问元素
2. 主要在尾部插入删除
3. 元素数量大致可预估
4. 需要连续内存

不太适合：

1. 频繁在头部或中间插入删除
2. 元素极大且完全无法预估，扩容代价非常敏感
3. 对元素地址长期稳定性要求很高

### 面试官可能追问

#### Q1：为什么选择倍数扩容，而不是固定大小扩容？

因为倍数扩容可以把多次扩容成本摊薄，让 `push_back` 的均摊复杂度保持在 `O(1)`。固定大小扩容会导致元素越来越多时扩容越来越频繁，总体效率更差。

#### Q2：如何避免频繁扩容带来的性能问题？

如果大概知道元素规模，提前用 `reserve()` 预留足够空间。

#### Q3：如何判断 vector 扩容策略？

可以通过 `capacity()` 观察增长模式，也可以查看具体 STL 实现源码。标准只规定语义，不规定固定倍数。

#### Q4：扩容时元素是拷贝还是移动？

如果元素类型支持高效移动构造，现代实现通常会优先移动；如果不支持，可能回退到拷贝。这也是为什么自定义类型的移动语义会影响 `vector` 扩容成本。

### 面试短答

> `vector` 底层是连续内存动态数组，通常维护起始位置、已用边界和容量边界。它支持 `O(1)` 随机访问，尾部插入均摊 `O(1)`，中间插入删除是 `O(n)`。当 `size == capacity` 时会触发扩容：申请更大内存，把旧元素搬过去，释放旧内存，再更新内部边界。扩容通常按倍数增长，这样能把 `push_back` 的均摊复杂度控制在 `O(1)`。如果提前知道元素规模，最好用 `reserve` 减少频繁扩容。

- `size` 和 `capacity`
- 扩容倍率
- 为什么扩容会导致迭代器失效
- 拷贝搬迁和移动搬迁
- `reserve` 和 `resize` 的区别

## push_back 和 emplace_back 的区别

### 简要回答

- `push_back` 是把一个已有对象压进去
- `emplace_back` 是把构造参数传进去，在容器尾部原地构造对象

更直白一点说：

- `push_back`：先在容器外把东西做好，再放进容器里。
- `emplace_back`：直接在容器内部把对象构造出来。

所以在很多场景下，`emplace_back` 可以少一次临时对象构造或少一次拷贝 / 移动。

### 详细回答

#### 1. push_back 做了什么

`push_back` 接收的是一个已经存在的对象。

```cpp
std::vector<std::string> v;
std::string s = "hello";
v.push_back(s);
```

这里的 `s` 已经在容器外构造好了，`push_back` 再把它复制或移动到容器尾部。

也就是说，`push_back` 的核心是：

- 先有对象
- 再把对象放进容器

#### 2. emplace_back 做了什么

`emplace_back` 接收的是构造参数，而不是现成对象。

```cpp
std::vector<std::string> v;
v.emplace_back("hello");
```

这里它会直接在容器尾部用参数 `"hello"` 原地构造 `std::string`。

也就是说，`emplace_back` 的核心是：

- 不先造中间对象
- 直接在容器内部构造目标对象

#### 3. 为什么 emplace_back 常常更高效

因为它通常能省掉：

- 一个临时对象构造
- 一次额外拷贝
- 或一次额外移动

尤其当元素类型构造比较重、参数比较复杂时，`emplace_back` 的优势更明显。

### 例子

#### push_back

```cpp
std::vector<std::pair<int, std::string>> v;
v.push_back(std::make_pair(1, "Alice"));
```

这里会先生成一个 `pair` 临时对象，再把它放进容器。

#### emplace_back

```cpp
std::vector<std::pair<int, std::string>> v;
v.emplace_back(1, "Alice");
```

这里会直接在容器尾部构造出 `pair<int, std::string>`。

#### 结合移动构造看差别

```cpp
#include <iostream>
#include <string>
#include <vector>
using namespace std;

class Person {
public:
    Person(string name, int age) : name_(std::move(name)), age_(age) {
        cout << "构造 Person: " << name_ << "\n";
    }

    Person(const Person& other) : name_(other.name_), age_(other.age_) {
        cout << "拷贝 Person: " << name_ << "\n";
    }

    Person(Person&& other) noexcept : name_(std::move(other.name_)), age_(other.age_) {
        cout << "移动 Person: " << name_ << "\n";
    }

private:
    string name_;
    int age_;
};

int main() {
    vector<Person> people;

    Person p1("Alice", 30);
    people.push_back(p1);                  // 已有对象，通常触发拷贝
    people.push_back(Person("Bob", 25));   // 临时对象，通常更容易触发移动

    people.emplace_back("Charlie", 40);    // 直接原地构造
    people.emplace_back(string("David"), 35); // 直接原地构造

    return 0;
}
```

这个例子很适合拿来观察：

- `push_back(p1)`：因为 `p1` 是现成左值对象，所以通常走拷贝构造。
- `push_back(Person(...))`：先生成临时对象，再放进容器，通常会触发移动构造。
- `emplace_back(...)`：直接把参数转给构造函数，在容器尾部原地构造对象。

相关知识：[[../C++/02 类与对象#std move 和移动语义|std::move 和移动语义]]

### 参数传递区别

| 接口 | 传入什么 | 特点 |
|---|---|---|
| `push_back` | 一个已有对象 | 语义直观 |
| `emplace_back` | 构造参数 | 直接原地构造 |

这也是为什么 `emplace_back` 更适合“参数多、构造复杂”的对象。

### 是否一定更快

不一定。

这是面试里很容易被追问的点。

#### 情况 1：已有现成对象

如果你已经有了一个对象：

```cpp
std::string s = "hello";
v.push_back(s);
```

这时候用 `push_back` 往往更自然，也不一定更差。

如果写成：

```cpp
v.emplace_back(s);
```

本质上还是要用 `s` 去构造容器里的对象，收益不一定明显。

#### 情况 2：vector 扩容

即使是 `emplace_back`，如果 `vector` 自己容量不够了，还是会触发扩容。

一旦扩容：

- 旧元素仍然要搬家
- 可能仍然涉及拷贝或移动

所以 `emplace_back` 不是“永远完全没有移动和拷贝”。

### 完美转发

`emplace_back` 背后的核心技术是完美转发。

它会把你传进去的参数，尽量以最合适的值类别和类型传给元素构造函数。

面试里不需要展开太深，知道它是“直接把参数转交给构造函数”的关键机制就够了。

### 对不同类型的效果差别

#### 对基础类型

比如：

```cpp
std::vector<int> v;
v.push_back(1);
v.emplace_back(2);
```

这种场景差别几乎不大。

#### 对复杂对象

如果对象构造参数多、内部资源多、临时对象代价高，`emplace_back` 通常更有价值。

### 什么时候用哪一个

一个很好记的原则：

#### 有现成对象，用 push_back

```cpp
Widget w(...);
vec.push_back(w);
```

#### 需要现场构造，用 emplace_back

```cpp
vec.emplace_back(arg1, arg2, arg3);
```

### 可读性问题

`emplace_back` 有时候会让代码阅读者看不出到底在构造什么对象，尤其是参数多、构造重载复杂时。

这时候虽然它可能更“高效”，但可读性会下降。

所以工程里要平衡：

- 性能收益
- 代码可读性

### 面试官可能追问

#### Q1：在什么情况下 push_back 可能比 emplace_back 更合适？

当你已经有现成对象时，`push_back` 语义更直接，代码也更清晰。并不是所有场景都要强行追求 `emplace_back`。

#### Q2：为什么 emplace_back 有时会让代码更难理解？

因为它接受的是构造参数，不是现成对象。看代码的人有时必须回头看类构造函数，才能知道最终构造出来的对象到底是什么样。

#### Q3：emplace_back 是否总是避免拷贝或移动？

不总是。

它能避免“新插入这个元素”的临时对象构造，但如果容器本身发生扩容，已有元素仍然需要搬迁。

### 面试短答

> `push_back` 是把一个已经存在的对象放进容器尾部，通常会发生一次拷贝或移动；`emplace_back` 是把构造参数直接传进去，在容器尾部原地构造对象，常常能少一次临时对象构造，所以对复杂对象通常更高效。  
> 不过它并不一定总更快：如果已经有现成对象，`push_back` 更直观；如果 `vector` 发生扩容，已有元素仍然要搬迁。所以一个比较实用的原则是：有现成对象用 `push_back`，需要现场构造用 `emplace_back`。

## 第二轮预留区

- `vector` vs `list` vs `deque`
- `set` vs `unordered_set`
- `reserve` vs `resize`
- 红黑树为什么不是 AVL
- 哈希冲突解决方式
