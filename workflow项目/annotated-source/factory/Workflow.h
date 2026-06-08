/*
  注释版源码文件：Workflow.h

  原始文件位置：
  workflow-master/src/factory/Workflow.h

  本文件用途：
  定义 Workflow、SeriesWork、ParallelWork。

  对你的随机图库项目来说：
  1. SeriesWork 用来表达“HTTP 请求 -> Redis 查询 -> MySQL 查询 -> File IO -> 返回响应”。
  2. ParallelWork 可以用来表达“同时读取多张图片 / 同时发多个请求 / 并行执行多个子流程”。
  3. Workflow 是创建和启动串行、并行工作流的静态工具类。
*/

#ifndef _WORKFLOW_H_                                          // 头文件保护宏：防止重复 include。
#define _WORKFLOW_H_                                          // 定义头文件保护宏。

#include <assert.h>                                           // 引入 assert；用于调试期检查任务是否处于合法状态。
#include <stddef.h>                                           // 引入 size_t、NULL 等基础定义。
#include <utility>                                            // 引入 std::move；用于移动 callback，避免不必要拷贝。
#include <functional>                                         // 引入 std::function；Workflow 的 callback 用它保存。
#include <mutex>                                              // 引入 std::mutex；SeriesWork 的任务队列需要加锁保护。
#include "SubTask.h"                                          // 引入 SubTask/ParallelTask；工作流本质上管理 SubTask。

class SeriesWork;                                             // 前置声明 SeriesWork；下面 callback 类型会用到。
class ParallelWork;                                           // 前置声明 ParallelWork；下面 callback 类型会用到。

using series_callback_t = std::function<void (const SeriesWork *)>;     // 串行流结束回调类型；参数是完成的 SeriesWork 指针。
using parallel_callback_t = std::function<void (const ParallelWork *)>; // 并行流结束回调类型；参数是完成的 ParallelWork 指针。

class Workflow                                                 // Workflow 是静态工厂类；它不保存状态，只负责创建/启动工作流。
{
public:                                                        // public 区域：用户常用的创建和启动接口。
	static SeriesWork *                                        // 返回值：新创建的 SeriesWork 指针，用户可以继续 push_back 追加任务。
	create_series_work(SubTask *first,                         // 输入参数 first：串行流里的第一个任务。
					   series_callback_t callback);            // 输入参数 callback：整个串行流结束时调用的回调。

	static void                                                // 返回值 void：创建后立即启动，所以不返回 SeriesWork 给用户。
	start_series_work(SubTask *first,                          // 输入参数 first：要启动的第一个任务。
					  series_callback_t callback);             // 输入参数 callback：串行流结束回调。

	static ParallelWork *                                      // 返回值：空的 ParallelWork，用户可以后续 add_series。
	create_parallel_work(parallel_callback_t callback);         // 输入参数 callback：并行组全部完成后调用。

	static ParallelWork *                                      // 返回值：包含多个子串行流的 ParallelWork。
	create_parallel_work(SeriesWork *const all_series[],        // 输入参数 all_series：子串行流数组，每个元素是一条并行分支。
						 size_t n,                              // 输入参数 n：子串行流数量。
						 parallel_callback_t callback);         // 输入参数 callback：并行组结束回调。

	static void                                                // 返回值 void：创建并立即启动并行流。
	start_parallel_work(SeriesWork *const all_series[],         // 输入参数 all_series：要并发执行的子串行流数组。
						size_t n,                               // 输入参数 n：子串行流数量。
						parallel_callback_t callback);          // 输入参数 callback：并行组完成回调。

public:                                                        // public 区域：带 last 任务的串行流创建接口。
	static SeriesWork *                                        // 返回值：新创建的 SeriesWork。
	create_series_work(SubTask *first,                         // 输入参数 first：第一个任务。
					   SubTask *last,                          // 输入参数 last：队列为空后最后执行的任务。
					   series_callback_t callback);            // 输入参数 callback：串行流结束回调。

	static void                                                // 返回值 void：创建并立即启动。
	start_series_work(SubTask *first,                          // 输入参数 first：第一个任务。
					  SubTask *last,                           // 输入参数 last：最后任务。
					  series_callback_t callback);             // 输入参数 callback：结束回调。
};

class SeriesWork                                               // SeriesWork 表示串行工作流，表达“任务 A 完成后执行任务 B”。
{
public:                                                        // public 区域：启动和放弃接口。
	void start()                                               // start()：启动当前串行流。
	{
		assert(!this->in_parallel);                            // 断言当前 SeriesWork 不在 ParallelWork 中；子并行流由 ParallelTask 启动。
		this->first->dispatch();                               // 启动第一个任务；后续任务由 subtask_done()/pop() 自动推进。
	}

	void dismiss()                                             // dismiss()：放弃一个已经创建但不打算启动的串行流。
	{
		assert(!this->in_parallel);                            // 断言只能在根串行流上调用；并行内部的子流由 ParallelWork 管理。
		this->dismiss_recursive();                             // 递归删除当前串行流里尚未执行的任务，释放资源。
	}

public:                                                        // public 区域：向串行流追加任务。
	void push_back(SubTask *task);                             // push_back(task)：把 task 加到队尾，当前任务完成后按顺序执行。
	void push_front(SubTask *task);                            // push_front(task)：把 task 插到队头，优先作为下一个任务执行。

public:                                                        // public 区域：上下文接口。
	void *get_context() const { return this->context; }         // get_context()：获取用户自定义上下文指针。
	void set_context(void *context) { this->context = context; } // set_context(context)：保存用户上下文，比如请求级数据结构。

public:                                                        // public 区域：取消和状态查询。
	virtual void cancel() { this->canceled = true; }            // cancel()：标记串行流取消；后续未执行任务会被清理。

	bool is_canceled() const { return this->canceled; }         // is_canceled()：返回串行流是否已经被取消。

	bool is_finished() const { return this->finished; }         // is_finished()：返回串行流是否已经走到结束回调阶段。

public:                                                        // public 区域：设置结束回调。
	void set_callback(series_callback_t callback)               // set_callback(callback)：替换当前串行流结束回调。
	{
		this->callback = std::move(callback);                   // 使用 std::move 保存 callback，避免复制 std::function 内部对象。
	}

public:                                                        // public 区域：扩展接口。
	virtual void *get_specific(const char *key) { return NULL; } // get_specific(key)：给子类扩展用；默认不支持任何 key。

public:                                                        // public 区域：主要给 task 实现内部调用。
	SubTask *pop();                                            // pop()：取出下一个要执行的任务；如果取消则清理剩余任务。

	SubTask *get_last_task() const { return this->last; }       // get_last_task()：获取最后任务指针。

	void set_last_task(SubTask *last)                           // set_last_task(last)：设置队列为空后最后执行的任务。
	{
		last->set_pointer(this);                                // 把 last 的 pointer 指向当前 SeriesWork，便于 series_of(last) 找回流程。
		this->last = last;                                      // 保存 last；pop_task() 在普通队列空后会取它。
	}

	void unset_last_task() { this->last = NULL; }               // unset_last_task()：清空最后任务，避免后续执行它。

	const ParallelTask *get_in_parallel() const                 // get_in_parallel()：查询当前串行流是否属于某个 ParallelTask。
	{
		return this->in_parallel;                               // 返回 in_parallel；NULL 表示不是并行组子流程。
	}

protected:                                                     // protected 区域：子类和 ParallelWork 可用。
	void set_in_parallel(const ParallelTask *task)              // set_in_parallel(task)：设置当前串行流所属的并行任务。
	{
		this->in_parallel = task;                               // 保存并行父任务指针。
	}

	void dismiss_recursive();                                  // dismiss_recursive()：递归删除串行流内尚未执行的任务。

protected:                                                     // protected 区域：用户上下文和回调。
	void *context;                                             // context：用户自定义上下文；可保存请求状态、临时数据等。
	series_callback_t callback;                                // callback：当前串行流完成时执行的回调。

private:                                                       // private 区域：内部队列操作。
	SubTask *pop_task();                                       // pop_task()：真正从队列中取任务；不处理 cancel 逻辑。
	void expand_queue();                                       // expand_queue()：当循环队列满时扩容。

private:                                                       // private 区域：串行流队列和状态。
	SubTask *buf[4];                                           // buf：小型内置队列，默认可容纳 4 个任务，避免一开始就动态分配。
	SubTask *first;                                            // first：串行流第一个任务；start() 会调用 first->dispatch()。
	SubTask *last;                                             // last：普通任务队列为空后最后执行的任务。
	SubTask **queue;                                           // queue：当前使用的任务队列指针；初始指向 buf，扩容后指向堆内存。
	int queue_size;                                            // queue_size：当前队列容量。
	int front;                                                 // front：循环队列头下标，pop_task() 从这里取任务。
	int back;                                                  // back：循环队列尾下标，push_back() 从这里写任务。
	bool canceled;                                             // canceled：是否已经取消当前串行流。
	bool finished;                                             // finished：是否已经完成当前串行流。
	const ParallelTask *in_parallel;                           // in_parallel：如果当前 SeriesWork 是并行组子流程，指向父 ParallelTask。
	std::mutex mutex;                                          // mutex：保护 queue/front/back/last 等成员，避免多线程追加或弹出任务时竞争。

protected:                                                     // protected 区域：构造和析构只允许 Workflow/ParallelWork 创建和销毁。
	SeriesWork(SubTask *first, series_callback_t&& callback);   // 构造函数：输入第一个任务和结束回调。
	virtual ~SeriesWork();                                     // 析构函数：释放扩容后的队列内存。
	friend class ParallelWork;                                 // ParallelWork 需要访问 SeriesWork 内部字段。
	friend class Workflow;                                     // Workflow 工厂需要调用受保护构造函数。
};

static inline SeriesWork *series_of(const SubTask *task)       // series_of(task)：根据任务指针找到所属 SeriesWork。
{
	return (SeriesWork *)task->get_pointer();                  // 通过 SubTask::pointer 取回 SeriesWork；这里做强制类型转换。
}

static inline SeriesWork& operator *(const SubTask& task)      // operator*：语法糖，可以通过 *task 拿到所属 SeriesWork 引用。
{
	return *series_of(&task);                                  // 先取 task 地址，再通过 series_of 找到 SeriesWork，最后解引用。
}

static inline SeriesWork& operator << (SeriesWork& series, SubTask *task) // operator<<：语法糖，支持 series << task。
{
	series.push_back(task);                                    // 把 task 追加到 series 队尾。
	return series;                                             // 返回 series 引用，便于连续 << 多个任务。
}

inline SeriesWork *Workflow::create_series_work(SubTask *first, series_callback_t callback) // 创建串行流但不启动。
{
	return new SeriesWork(first, std::move(callback));          // new 一个 SeriesWork，保存 first 和 callback，返回给用户继续配置。
}

inline void Workflow::start_series_work(SubTask *first, series_callback_t callback) // 创建并启动串行流。
{
	new SeriesWork(first, std::move(callback));                 // 创建 SeriesWork；对象后续会在流程结束时自动 delete。
	first->dispatch();                                         // 立即启动第一个任务。
}

inline SeriesWork *Workflow::create_series_work(SubTask *first, SubTask *last,
												series_callback_t callback) // 创建带 last 任务的串行流。
{
	SeriesWork *series = new SeriesWork(first, std::move(callback)); // 创建串行流对象。
	series->set_last_task(last);                                // 设置最后任务；普通队列为空后会执行 last。
	return series;                                              // 返回创建好的串行流。
}

inline void Workflow::start_series_work(SubTask *first, SubTask *last,
										series_callback_t callback) // 创建并启动带 last 任务的串行流。
{
	SeriesWork *series = new SeriesWork(first, std::move(callback)); // 创建串行流对象。
	series->set_last_task(last);                                // 设置最后任务。
	first->dispatch();                                          // 启动第一个任务。
}

class ParallelWork : public ParallelTask                        // ParallelWork 是面向用户的并行工作流，底层继承 ParallelTask。
{
public:                                                         // public 区域：启动和放弃接口。
	void start()                                                // start()：启动并行工作流。
	{
		assert(!series_of(this));                               // 断言当前 ParallelWork 还不属于任何 SeriesWork。
		Workflow::start_series_work(this, nullptr);             // 把 ParallelWork 自己作为一个任务放进 SeriesWork 启动。
	}

	void dismiss()                                              // dismiss()：放弃一个未启动的并行工作流。
	{
		assert(!series_of(this));                               // 断言只能在未挂入其他流程时 dismiss。
		delete this;                                            // 删除当前 ParallelWork；析构会清理内部子流程。
	}

public:                                                         // public 区域：添加子串行流。
	void add_series(SeriesWork *series);                        // add_series(series)：把一条 SeriesWork 加入并行组。

public:                                                         // public 区域：上下文接口。
	void *get_context() const { return this->context; }          // get_context()：获取用户上下文。
	void set_context(void *context) { this->context = context; } // set_context(context)：设置用户上下文。

public:                                                         // public 区域：访问子流程。
	SeriesWork *series_at(size_t index)                         // series_at(index)：按下标获取子串行流。
	{
		if (index < this->subtasks_nr)                          // 如果 index 在合法范围内。
			return this->all_series[index];                     // 返回对应子串行流。
		else                                                    // 如果 index 越界。
			return NULL;                                        // 返回 NULL 表示不存在。
	}

	const SeriesWork *series_at(size_t index) const             // const 版本的 series_at(index)。
	{
		if (index < this->subtasks_nr)                          // 检查下标是否小于子任务数量。
			return this->all_series[index];                     // 返回对应子流程。
		else                                                    // 越界情况。
			return NULL;                                        // 返回 NULL。
	}

	SeriesWork& operator[] (size_t index) { return *this->all_series[index]; } // operator[]：以引用方式访问子流程。

	const SeriesWork& operator[] (size_t index) const            // const 版本 operator[]。
	{
		return *this->all_series[index];                        // 返回对应子流程引用；这里不做越界检查。
	}

	size_t size() const { return this->subtasks_nr; }            // size()：返回并行组里子串行流数量。

public:                                                         // public 区域：迭代器风格接口。
	SeriesWork *const *begin() { return this->all_series; }      // begin()：返回子流程数组起始地址。
	SeriesWork *const *end() { return this->all_series + this->subtasks_nr; } // end()：返回子流程数组结束地址。

	const SeriesWork *const *begin() const                       // const begin()。
	{
		return (const SeriesWork **)this->all_series;            // 转成 const 指针返回。
	}

	const SeriesWork *const *end() const                         // const end()。
	{
		return (const SeriesWork **)this->all_series + this->subtasks_nr; // 返回 const 结束位置。
	}

public:                                                         // public 区域：设置并行结束回调。
	void set_callback(parallel_callback_t callback)              // set_callback(callback)：替换并行工作流结束回调。
	{
		this->callback = std::move(callback);                    // 移动保存 callback，避免不必要拷贝。
	}

protected:                                                      // protected 区域：实现 SubTask 完成逻辑。
	virtual SubTask *done();                                     // done()：并行组全部完成后的收尾函数，返回后续任务。

protected:                                                      // protected 区域：上下文和回调。
	void *context;                                              // context：用户自定义上下文。
	parallel_callback_t callback;                               // callback：并行组完成回调。

private:                                                        // private 区域：内部数组扩容。
	void expand_buf();                                          // expand_buf()：当子流程数组容量不足时扩容。

private:                                                        // private 区域：并行组数组信息。
	size_t buf_size;                                            // buf_size：当前数组容量。
	SeriesWork **all_series;                                    // all_series：保存所有子 SeriesWork 的数组。

protected:                                                      // protected 区域：构造和析构只允许 Workflow 创建。
	ParallelWork(parallel_callback_t&& callback);               // 构造空并行流。
	ParallelWork(SeriesWork *const all_series[], size_t n,
				 parallel_callback_t&& callback);               // 构造包含 n 条子流程的并行流。
	virtual ~ParallelWork();                                    // 析构函数：清理子流程和数组。
	friend class Workflow;                                      // Workflow 可以调用受保护构造函数。
};

inline ParallelWork *Workflow::create_parallel_work(parallel_callback_t callback) // 创建空并行流。
{
	return new ParallelWork(std::move(callback));                // new 一个 ParallelWork 并返回给用户。
}

inline ParallelWork *Workflow::create_parallel_work(SeriesWork *const all_series[], size_t n,
												   parallel_callback_t callback) // 创建包含多条子流程的并行流。
{
	return new ParallelWork(all_series, n, std::move(callback)); // 创建 ParallelWork，内部会记录每条子 SeriesWork。
}

inline void Workflow::start_parallel_work(SeriesWork *const all_series[], size_t n,
										  parallel_callback_t callback) // 创建并立即启动并行流。
{
	ParallelWork *p = new ParallelWork(all_series, n, std::move(callback)); // 创建并行工作流对象。
	Workflow::start_series_work(p, nullptr);                 // 把 ParallelWork 作为一个 SubTask 放进串行流启动。
}

#endif                                                        // 结束头文件保护宏。
