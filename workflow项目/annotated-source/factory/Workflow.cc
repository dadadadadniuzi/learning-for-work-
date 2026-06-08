/*
  注释版源码文件：Workflow.cc

  原始文件位置：
  workflow-master/src/factory/Workflow.cc

  本文件用途：
  实现 SeriesWork 和 ParallelWork 的具体逻辑。

  重点理解：
  1. SeriesWork 内部是一个线程安全的循环队列。
  2. 当前任务完成后，会调用 series->pop() 取出下一个任务。
  3. ParallelWork 内部保存多条 SeriesWork，每条 SeriesWork 的 first 任务作为一个并行分支入口。
*/

#include <assert.h>                                           // 引入 assert；用于运行时检查内部状态。
#include <stddef.h>                                           // 引入 size_t、NULL 等基础定义。
#include <string.h>                                           // 引入 memcpy；ParallelWork 扩容时会复制数组。
#include <utility>                                            // 引入 std::move；用于移动 callback。
#include <functional>                                         // 引入 std::function；callback 类型依赖它。
#include <mutex>                                              // 引入 std::mutex；SeriesWork 队列操作需要互斥。
#include "Workflow.h"                                         // 引入 SeriesWork、ParallelWork 的声明。

SeriesWork::SeriesWork(SubTask *first, series_callback_t&& cb) : // SeriesWork 构造函数。
	callback(std::move(cb))                                    // 初始化 callback；用 std::move 接管传入回调。
{
	this->queue = this->buf;                                   // queue 初始指向内置小数组 buf，避免少量任务时动态分配。
	this->queue_size = sizeof this->buf / sizeof *this->buf;   // queue_size 计算 buf 可容纳多少个 SubTask*；这里通常是 4。
	this->front = 0;                                           // front 初始化为 0，表示队列头从下标 0 开始。
	this->back = 0;                                            // back 初始化为 0，表示队列尾也从下标 0 开始；front==back 表示空队列。
	this->canceled = false;                                    // canceled 初始化为 false，表示串行流未取消。
	this->finished = false;                                    // finished 初始化为 false，表示串行流还没有执行结束。
	assert(!series_of(first));                                 // 检查 first 还没有归属任何 SeriesWork，避免一个任务被放进多个流程。
	first->set_pointer(this);                                  // 把 first 的 pointer 指向当前 SeriesWork，后续 series_of(first) 可找回流程。
	this->first = first;                                       // 保存第一个任务指针；start() 会启动它。
	this->last = NULL;                                         // last 初始化为 NULL，表示没有额外的最后任务。
	this->context = NULL;                                      // context 初始化为 NULL，用户尚未设置请求级上下文。
	this->in_parallel = NULL;                                  // in_parallel 初始化为 NULL，表示当前串行流不是并行组的子流程。
}

SeriesWork::~SeriesWork()                                     // SeriesWork 析构函数。
{
	if (this->queue != this->buf)                              // 如果 queue 不再指向内置 buf，说明曾经扩容到堆内存。
		delete []this->queue;                                  // 释放堆上分配的任务队列数组。
}

void SeriesWork::dismiss_recursive()                          // dismiss_recursive()：递归删除当前流程中未执行任务。
{
	SubTask *task = first;                                     // task：当前要删除的任务，初始为 first。

	this->callback = nullptr;                                  // 清空 callback；放弃流程时不应该再触发用户结束回调。
	do                                                        // do-while：至少删除 first。
	{
		delete task;                                          // 删除当前任务对象；释放它占用的资源。
		task = this->pop_task();                              // 继续取下一个任务；pop_task() 会从队列或 last 中拿任务。
	} while (task);                                           // 只要还能取到任务，就继续删除。
}

void SeriesWork::expand_queue()                               // expand_queue()：扩容 SeriesWork 的循环队列。
{
	int size = 2 * this->queue_size;                           // size：新容量，扩成原容量的 2 倍。
	SubTask **queue = new SubTask *[size];                     // queue：新分配的任务指针数组，用于替换旧队列。
	int i, j;                                                  // i：新数组写入下标；j：旧循环队列读取下标。

	i = 0;                                                     // 新数组从 0 开始写。
	j = this->front;                                           // 从旧队列的 front 开始读，保持任务原有顺序。
	do                                                        // 循环复制旧队列中已有任务。
	{
		queue[i++] = this->queue[j++];                         // 把旧队列 j 位置任务复制到新队列 i 位置，然后两个下标都前进。
		if (j == this->queue_size)                             // 如果旧队列下标走到末尾。
			j = 0;                                             // 回绕到 0，因为旧队列是循环队列。
	} while (j != this->back);                                 // 直到读到旧队列 back 位置为止。

	if (this->queue != this->buf)                              // 如果旧队列是堆内存，而不是内置 buf。
		delete []this->queue;                                  // 释放旧队列内存，避免内存泄漏。

	this->queue = queue;                                       // queue 成员指向新数组。
	this->queue_size = size;                                   // 更新队列容量为新容量。
	this->front = 0;                                           // 新队列已经线性排列，所以 front 重置为 0。
	this->back = i;                                            // back 设置为已复制任务数量的位置。
}

void SeriesWork::push_front(SubTask *task)                    // push_front(task)：把任务插到队头，让它优先执行。
{
	this->mutex.lock();                                        // 加锁，保护 front/back/queue，避免多线程同时修改队列。
	if (--this->front == -1)                                   // front 向前移动一格；如果减到 -1，说明越过数组开头。
		this->front = this->queue_size - 1;                    // 回绕到数组最后一个位置，维持循环队列结构。

	task->set_pointer(this);                                   // 设置 task 所属 SeriesWork，后续任务完成时能找到当前流程。
	this->queue[this->front] = task;                           // 把 task 写入新的 front 位置。
	if (this->front == this->back)                             // 如果 front 追上 back，说明队列已经满了。
		this->expand_queue();                                  // 扩容队列，避免后续写入覆盖已有任务。

	this->mutex.unlock();                                      // 解锁，允许其他线程继续操作队列。
}

void SeriesWork::push_back(SubTask *task)                     // push_back(task)：把任务追加到队尾，按顺序稍后执行。
{
	this->mutex.lock();                                        // 加锁，保护循环队列。
	task->set_pointer(this);                                   // 设置 task 所属 SeriesWork。
	this->queue[this->back] = task;                            // 把 task 写入当前 back 位置。
	if (++this->back == this->queue_size)                      // back 前进一格；如果走到队列末尾。
		this->back = 0;                                        // 回绕到 0。

	if (this->front == this->back)                             // 如果 back 追上 front，说明队列满了。
		this->expand_queue();                                  // 扩容队列，保持任务不被覆盖。

	this->mutex.unlock();                                      // 解锁。
}

SubTask *SeriesWork::pop()                                    // pop()：取出下一个任务，并处理取消状态。
{
	bool canceled = this->canceled;                            // canceled：先保存取消状态，避免后续逻辑中状态变化造成混乱。
	SubTask *task = this->pop_task();                          // task：从普通队列或 last 中取出的下一个任务。

	if (!canceled)                                             // 如果当前串行流没有取消。
		return task;                                           // 直接返回下一个任务，交给 SubTask::subtask_done() 调度。

	while (task)                                               // 如果已经取消，则需要清理剩余所有任务。
	{
		delete task;                                           // 删除当前未执行任务。
		task = this->pop_task();                               // 继续取下一个未执行任务。
	}

	return NULL;                                               // 取消状态下不再返回后续任务。
}

SubTask *SeriesWork::pop_task()                               // pop_task()：实际从队列取任务，不负责 cancel 逻辑。
{
	SubTask *task;                                             // task：准备返回的下一个任务指针。

	this->mutex.lock();                                        // 加锁，保护队列读取。
	if (this->front != this->back)                             // 如果 front != back，说明普通任务队列不为空。
	{
		task = this->queue[this->front];                       // 从 front 位置取出任务。
		if (++this->front == this->queue_size)                 // front 前进一格；如果走到数组末尾。
			this->front = 0;                                   // 回绕到 0。
	}
	else                                                       // 如果普通任务队列为空。
	{
		task = this->last;                                     // 尝试取 last 任务；last 可能是服务端回复任务等收尾任务。
		this->last = NULL;                                     // 取出后清空 last，避免重复执行。
	}

	this->mutex.unlock();                                      // 解锁。
	if (!task)                                                 // 如果普通队列和 last 都没有任务，说明串行流结束。
	{
		this->finished = true;                                 // 标记当前串行流已经完成。

		if (this->callback)                                    // 如果用户设置了结束回调。
			this->callback(this);                              // 调用结束回调，参数是当前 SeriesWork 指针。

		if (!this->in_parallel)                                // 如果当前串行流不是 ParallelWork 的子流程。
			delete this;                                       // 删除当前 SeriesWork；根串行流生命周期到此结束。
	}

	return task;                                               // 返回取到的任务；可能为 NULL。
}

ParallelWork::ParallelWork(parallel_callback_t&& cb) :        // ParallelWork 空构造函数。
	ParallelTask(new SubTask *[2 * 4], 0),                     // 初始化父类 ParallelTask：分配一个数组，初始子任务数量为 0。
	callback(std::move(cb))                                    // 移动保存并行结束回调。
{
	this->buf_size = 4;                                        // buf_size 初始化为 4，表示初始可容纳 4 条子 SeriesWork。
	this->all_series = (SeriesWork **)&this->subtasks[this->buf_size]; // all_series 放在 subtasks 数组后半段，共用一块内存。
	this->context = NULL;                                      // context 初始化为 NULL，用户尚未设置并行组上下文。
}

ParallelWork::ParallelWork(SeriesWork *const all_series[], size_t n,
						   parallel_callback_t&& cb) :         // ParallelWork 带子流程构造函数。
	ParallelTask(new SubTask *[2 * (n > 4 ? n : 4)], n),        // 初始化父类：数组容量至少 4，子任务数量为 n。
	callback(std::move(cb))                                    // 移动保存 callback。
{
	size_t i;                                                  // i：遍历 all_series 数组的下标。

	this->buf_size = (n > 4 ? n : 4);                           // buf_size：容量至少 4；如果 n 更大，就用 n。
	this->all_series = (SeriesWork **)&this->subtasks[this->buf_size]; // all_series 指向同一块数组后半段。
	for (i = 0; i < n; i++)                                    // 遍历每条子串行流。
	{
		assert(!all_series[i]->in_parallel);                   // 确认该 SeriesWork 还没有加入其他并行组。
		all_series[i]->in_parallel = this;                     // 标记该子流程属于当前 ParallelWork。
		this->all_series[i] = all_series[i];                   // 保存子流程指针。
		this->subtasks[i] = all_series[i]->first;              // 保存子流程的第一个任务，ParallelTask::dispatch() 会启动它。
	}

	this->context = NULL;                                      // context 初始化为空。
}

void ParallelWork::expand_buf()                               // expand_buf()：扩容 ParallelWork 内部数组。
{
	SubTask **buf;                                             // buf：新分配的数组。
	size_t size;                                               // size：需要复制的字节数。

	this->buf_size *= 2;                                       // 容量翻倍。
	buf = new SubTask *[2 * this->buf_size];                   // 新数组大小是 2 * buf_size，因为前半存 subtasks，后半存 all_series。
	size = this->subtasks_nr * sizeof (void *);                // 计算当前已有元素占用的字节数。
	memcpy(buf, this->subtasks, size);                         // 复制前半段 subtasks。
	memcpy(buf + this->buf_size, this->all_series, size);       // 复制后半段 all_series 到新位置。

	delete []this->subtasks;                                   // 释放旧数组。
	this->subtasks = buf;                                      // subtasks 指向新数组前半段。
	this->all_series = (SeriesWork **)&buf[this->buf_size];     // all_series 指向新数组后半段。
}

void ParallelWork::add_series(SeriesWork *series)              // add_series(series)：向并行组加入一条子串行流。
{
	if (this->subtasks_nr == this->buf_size)                   // 如果当前子流程数量已经达到容量上限。
		this->expand_buf();                                    // 扩容内部数组。

	assert(!series->in_parallel);                              // 确认这条 SeriesWork 不属于其他 ParallelWork。
	series->in_parallel = this;                                // 标记它属于当前并行组。
	this->all_series[this->subtasks_nr] = series;               // 保存子 SeriesWork 指针。
	this->subtasks[this->subtasks_nr] = series->first;          // 保存该子流程的入口任务。
	this->subtasks_nr++;                                       // 子流程数量加 1。
}

SubTask *ParallelWork::done()                                  // done()：并行组全部完成后的收尾函数。
{
	SeriesWork *series = series_of(this);                      // series：当前 ParallelWork 自己所在的外层串行流。
	size_t i;                                                  // i：遍历子流程数组。

	if (this->callback)                                        // 如果设置了并行完成回调。
		this->callback(this);                                  // 调用并行完成回调。

	for (i = 0; i < this->subtasks_nr; i++)                    // 遍历所有子 SeriesWork。
		delete this->all_series[i];                            // 删除每条子流程；子流程本身已经完成，只需释放对象。

	this->subtasks_nr = 0;                                     // 把子流程数量置 0，避免析构时重复清理。
	delete this;                                               // 删除当前 ParallelWork 对象。
	return series->pop();                                      // 从外层串行流里取下一个任务，继续执行并行组后面的流程。
}

ParallelWork::~ParallelWork()                                  // ParallelWork 析构函数。
{
	size_t i;                                                  // i：遍历子流程。

	for (i = 0; i < this->subtasks_nr; i++)                    // 遍历所有尚未清理的子流程。
	{
		this->all_series[i]->in_parallel = NULL;               // 清空子流程的并行归属标记。
		this->all_series[i]->dismiss_recursive();              // 递归放弃并删除子流程中未执行任务。
	}

	delete []this->subtasks;                                   // 释放内部数组；all_series 与 subtasks 共用这块内存。
}
