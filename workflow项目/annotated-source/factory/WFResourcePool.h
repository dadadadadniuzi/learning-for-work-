/*
  注释版源码文件：WFResourcePool.h

  原始文件位置：
  workflow-master/src/factory/WFResourcePool.h

  本文件用途：
  WFResourcePool 是 Workflow 的资源池封装。

  关键理解：
  - 资源池里保存一组 void* 资源。
  - get() 获取资源；没有资源时可以让任务等待在 Conditional 上。
  - post() 归还资源；如果有任务在等，就唤醒等待任务。

  对随机图库项目的意义：
  - 你可以把它类比成“连接池/令牌池/并发限流池”。
  - Workflow 内部 DNS 资源池也通过 WFGlobal 暴露。
  - 面试时可用于解释：框架不只做异步 IO，也有任务级资源协调能力。
*/

#ifndef _WFRESOURCEPOOL_H_                                    // 头文件保护宏。
#define _WFRESOURCEPOOL_H_                                    // 定义头文件保护宏。

#include <mutex>                                              // std::mutex，保护资源池数据。
#include "list.h"                                             // list_head，保存等待任务链表。
#include "WFTask.h"                                           // WFConditional、SubTask 等任务控制对象。

class WFResourcePool                                          // WFResourcePool：资源池类。
{
public:
	WFConditional *get(SubTask *task, void **resbuf);           // get(task,resbuf)：为 task 获取资源，资源写入 resbuf；不满足时返回等待条件。
	WFConditional *get(SubTask *task);                          // get(task)：只申请资源名额，不直接返回资源指针。
	void post(void *res);                                       // post(res)：归还资源，并可能唤醒等待任务。

public:
	struct Data                                                // Data：资源池内部共享数据。
	{
		void *pop() { return this->pool->pop(); }               // pop()：从池中弹出一个资源，实际调用虚函数方便子类定制。
		void push(void *res) { this->pool->push(res); }         // push(res)：把资源放回池中，实际调用虚函数方便子类定制。

		void **res;                                            // res：资源数组，每个元素是一个 void* 资源。
		long value;                                            // value：当前可用资源数量或条件计数值。
		size_t index;                                          // index：资源数组栈顶位置，用于 pop/push。
		struct list_head wait_list;                            // wait_list：等待资源的任务链表。
		std::mutex mutex;                                      // mutex：保护 value、index、wait_list、res。
		WFResourcePool *pool;                                  // pool：指回所属资源池对象，用于调用虚函数 pop/push。
	};

protected:
	virtual void *pop()                                        // pop()：默认从数组中取出一个资源。
	{
		return this->data.res[this->data.index++];              // 返回当前 index 位置资源，然后 index 后移。
	}

	virtual void push(void *res)                                // push(res)：默认把资源放回数组。
	{
		this->data.res[--this->data.index] = res;               // index 前移一个位置，再写回资源。
	}

protected:
	struct Data data;                                           // data：资源池核心状态。

private:
	void create(size_t n);                                      // create(n)：创建 n 个资源槽位并初始化内部状态。

public:
	WFResourcePool(void *const *res, size_t n);                 // 构造函数：用外部已有资源数组初始化资源池。
	WFResourcePool(size_t n);                                  // 构造函数：只创建 n 个空资源槽位。
	virtual ~WFResourcePool() { delete []this->data.res; }      // 析构函数：释放资源指针数组；具体资源释放由使用者或子类负责。
};

#endif                                                        // 结束头文件保护。

