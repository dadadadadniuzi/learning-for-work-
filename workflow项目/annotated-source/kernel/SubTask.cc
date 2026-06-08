/*
  注释版源码文件：SubTask.cc

  原始文件位置：
  workflow-master/src/kernel/SubTask.cc

  本文件用途：
  这里实现了 Workflow 最核心的任务推进逻辑。
  你可以把 subtask_done() 理解成“任务完成后的调度中枢”。

  最关键的一句话：
  一个任务完成后，框架会调用 done() 拿到下一个任务，如果有下一个任务就继续 dispatch()；
  如果没有下一个任务，并且当前任务属于某个并行父任务，就通知父任务当前分支完成。
*/

#include "SubTask.h"                                      // 引入 SubTask 和 ParallelTask 的类声明。

void SubTask::subtask_done()                              // subtask_done()：当前任务完成时调用，用于推进后续任务。
{                                                         // 没有输入参数；因为当前任务就是 this。
	SubTask *cur = this;                                  // cur：当前正在处理的任务指针；初始值是 this，也就是刚完成的任务。
	ParallelTask *parent;                                 // parent：当前任务所属的并行父任务；用于处理并行分支完成计数。

	while (1)                                             // 无限循环：用于连续推进任务，直到没有可继续调度的任务为止。
	{                                                     // 为什么用循环：done() 可能返回下一个任务，下一个任务执行后可能又立即完成。
		parent = cur->parent;                             // 读取当前任务的 parent；保存下来是因为 done() 后 cur 会变成下一个任务。
		cur = cur->done();                                // 调用当前任务的 done() 收尾，并把返回值赋给 cur。
		                                                  // 如果返回非 NULL，cur 就是下一个需要启动的任务。
		                                                  // 如果返回 NULL，说明当前串行链没有下一个任务。

		if (cur)                                          // 如果 done() 返回了下一个任务，说明当前串行流程还没结束。
		{
			cur->parent = parent;                         // 把上一个任务的并行父任务传给下一个任务。
			                                              // 为什么要传 parent：如果这条串行链属于某个 ParallelTask，链上的后续任务也属于同一个并行分支。
			cur->dispatch();                              // 启动下一个任务；dispatch() 由具体任务实现，比如网络任务会发起异步请求。
		}
		else if (parent)                                  // 如果没有下一个任务，但当前任务属于某个并行父任务。
		{
			if (__sync_sub_and_fetch(&parent->nleft, 1) == 0) // 原子地把父任务的 nleft 减 1，并检查是否所有分支都完成。
			{                                             // __sync_sub_and_fetch 是 GCC 原子内置函数，避免多线程并发完成时计数出错。
				cur = parent;                             // 如果 nleft 减到 0，说明整个并行组完成；把 cur 切换成 parent。
				continue;                                 // continue：重新进入循环，让 parent 也执行 done()，从而推进并行组后面的任务。
			}
		}

		break;                                            // 没有下一个任务，或者并行组还没全部完成，就退出循环。
	}
}

void ParallelTask::dispatch()                             // ParallelTask::dispatch()：启动并行任务组中的所有子任务。
{                                                         // 没有输入参数；子任务数组和数量来自成员变量 subtasks/subtasks_nr。
	SubTask **end = this->subtasks + this->subtasks_nr;   // end：子任务数组的结束位置；等于首地址加任务数量。
	SubTask **p = this->subtasks;                         // p：遍历子任务数组用的指针；初始指向第一个子任务。

	this->nleft = this->subtasks_nr;                       // nleft 初始化为子任务数量；表示还有多少并行分支未完成。
	if (this->nleft != 0)                                  // 如果子任务数量不为 0，就逐个启动所有子任务。
	{
		do                                                // do-while：至少启动一次，因为这里已经确认 nleft != 0。
		{
			(*p)->parent = this;                          // 设置当前子任务的 parent 为当前 ParallelTask。
			                                              // 为什么要设置：子任务完成时要通过 parent->nleft 通知并行组完成进度。
			(*p)->dispatch();                             // 启动当前子任务；每个子任务可能是一个串行流的第一个任务。
		} while (++p != end);                             // p 后移到下一个子任务；如果还没到 end，就继续启动。
	}
	else                                                  // 如果并行组里没有任何子任务。
		this->subtask_done();                             // 空并行组直接视为完成，调用 subtask_done() 推进后续流程。
}
