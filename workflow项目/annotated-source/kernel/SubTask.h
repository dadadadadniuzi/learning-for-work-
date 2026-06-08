/*
  注释版源码文件：SubTask.h

  原始文件位置：
  workflow-master/src/kernel/SubTask.h

  本文件用途：
  这是 Workflow 框架最底层的任务抽象定义文件。
  所有 HTTP、Redis、MySQL、Timer、File IO 等任务，最终都可以被看成某种 SubTask。

  学习重点：
  1. SubTask 定义了“一个任务如何启动、如何结束、如何交接下一个任务”。
  2. ParallelTask 定义了“多个任务如何并行启动，并在全部结束后继续往下走”。
  3. 这里是理解 SeriesWork / ParallelWork / WFTask 的起点。
*/

#ifndef _SUBTASK_H_                 // 头文件保护宏：防止同一个头文件被重复 include，导致类重复定义。
#define _SUBTASK_H_                 // 定义头文件保护宏，后续再次 include 时会跳过本文件内容。

#include <stddef.h>                 // 引入 size_t 和 NULL 等基础定义；ParallelTask 里会用 size_t 表示任务数量。

class SubTask                       // SubTask 是 Workflow 中最小的任务抽象；任何可调度任务都要继承它。
{
public:                             // public 区域：外部或框架调度器可以调用的接口。
	virtual void dispatch() = 0;     // dispatch()：启动任务；纯虚函数，说明每种具体任务都必须自己实现“怎么启动”。

private:                            // private 区域：只允许 SubTask 内部或友元机制间接使用。
	virtual SubTask *done() = 0;     // done()：当前任务完成后的收尾函数；返回值是下一个要执行的 SubTask 指针。
	                                // 返回非 NULL：框架会继续调度这个下一个任务。
	                                // 返回 NULL：当前串行流程可能结束，或当前并行分支结束。

protected:                          // protected 区域：子类可以调用，但普通用户不能直接调用。
	void subtask_done();             // subtask_done()：任务完成时调用的统一入口；它会调用 done() 并推进后续任务。

public:                             // public 区域：提供一个通用指针，用来把任务挂到某个上下文上。
	void *get_pointer() const        // get_pointer()：获取 pointer 指针；通常用于拿到当前任务所属的 SeriesWork。
	{
		return this->pointer;        // 返回当前任务保存的上下文指针；this->pointer 一般由 SeriesWork 设置。
	}

	void set_pointer(void *pointer)  // set_pointer(pointer)：设置任务上下文指针。
	{                               // 输入参数 pointer：外部传入的上下文地址，类型是 void*，可以保存任意对象地址。
		this->pointer = pointer;     // 把参数 pointer 保存到成员变量 this->pointer，后续 series_of(task) 会读取它。
	}

private:                            // private 区域：任务调度内部状态，普通用户不应该直接操作。
	class ParallelTask *parent;      // parent：如果当前任务属于一个 ParallelTask，则 parent 指向那个并行父任务。
	                                // 作用：当前并行子任务结束时，需要通知父并行任务“我完成了”。
	void *pointer;                   // pointer：通用上下文指针；在 Workflow 中通常指向当前任务所属的 SeriesWork。
	                                // 为什么用 void*：底层任务抽象不想依赖 factory 层的 SeriesWork 类型。

public:                             // public 区域：构造和析构函数。
	SubTask()                       // SubTask 构造函数：创建一个任务时初始化调度状态。
	{
		this->parent = NULL;         // parent 初始化为 NULL，表示默认不属于任何并行父任务。
		this->pointer = NULL;        // pointer 初始化为 NULL，表示任务暂时还没有挂到任何 SeriesWork 或上下文。
	}

	virtual ~SubTask() { }           // 虚析构函数：保证通过 SubTask* 删除子类对象时，会正确调用子类析构函数。
	friend class ParallelTask;       // 声明 ParallelTask 为友元类；ParallelTask 可以访问 parent、done() 等私有成员。
};

class ParallelTask : public SubTask  // ParallelTask 继承 SubTask；它本身也是一个任务，但内部包含多个子任务。
{
public:                             // public 区域：实现从 SubTask 继承来的启动接口。
	virtual void dispatch();         // dispatch()：启动并行任务；它会把 subtasks 数组里的所有子任务都启动起来。

protected:                          // protected 区域：子类 ParallelWork 可以访问这些并行任务数组信息。
	SubTask **subtasks;              // subtasks：子任务数组；每个元素都是一个 SubTask*，表示一个并行分支的入口任务。
	size_t subtasks_nr;              // subtasks_nr：子任务数量；告诉 dispatch() 一共有多少个并行分支要启动。

private:                            // private 区域：并行完成计数，只由 ParallelTask/SubTask 调度逻辑使用。
	size_t nleft;                    // nleft：还没有完成的子任务数量；每个子任务完成时会原子减 1。
	                                // 当 nleft 减到 0，说明并行组全部完成，可以执行 ParallelTask::done()。

public:                             // public 区域：构造和析构函数。
	ParallelTask(SubTask **subtasks, size_t n) // 构造函数：创建一个并行任务。
	{                               // 输入参数 subtasks：并行子任务数组。
	                                // 输入参数 n：并行子任务数量。
		this->subtasks = subtasks;   // 保存子任务数组地址；dispatch() 会遍历这个数组启动任务。
		this->subtasks_nr = n;       // 保存子任务数量；用于计算数组边界和初始化 nleft。
	}

	virtual ~ParallelTask() { }      // 虚析构函数：允许通过基类指针安全删除并行任务对象。
	friend class SubTask;            // 声明 SubTask 为友元类；SubTask::subtask_done() 需要访问 nleft。
};

#endif                              // 结束头文件保护宏。
