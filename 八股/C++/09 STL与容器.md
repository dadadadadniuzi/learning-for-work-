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
