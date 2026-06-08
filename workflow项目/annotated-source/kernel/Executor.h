/*
  注释版源码文件：Executor.h

  原始文件位置：
  workflow-master/src/kernel/Executor.h

  本文件用途：
  Executor 是 Workflow 的计算任务线程池抽象。

  对高并发面试最重要：
  1. 网络 IO 走 Communicator/CommScheduler。
  2. 计算任务走 Executor。
  3. ExecRequest 把计算任务包装成 SubTask，因此计算任务也能放进 SeriesWork。
*/

#ifndef _EXECUTOR_H_                                          // 头文件保护宏。
#define _EXECUTOR_H_                                          // 定义头文件保护宏。

#include <stddef.h>                                           // size_t。
#include <pthread.h>                                          // pthread_mutex_t。
#include "list.h"                                             // 内核风格链表。

class ExecQueue                                               // ExecQueue：执行队列，同一队列中的任务通常按队列调度。
{
public:
	int init();                                                // init()：初始化队列链表和锁。
	void deinit();                                             // deinit()：销毁队列资源。

private:
	struct list_head session_list;                             // session_list：等待执行的 ExecSession 链表。
	pthread_mutex_t mutex;                                     // mutex：保护 session_list。

public:
	virtual ~ExecQueue() { }                                   // 虚析构函数。
	friend class Executor;                                     // Executor 需要访问队列内部链表。
};

#define ES_STATE_FINISHED	0                                  // 计算任务完成。
#define ES_STATE_ERROR		1                                  // 计算任务发生错误。
#define ES_STATE_CANCELED	2                                  // 计算任务被取消。

class ExecSession                                             // ExecSession：一个可被 Executor 执行的计算会话。
{
private:
	virtual void execute() = 0;                                // execute()：真正在线程池中运行的计算逻辑。
	virtual void handle(int state, int error) = 0;              // handle(state,error)：计算完成/失败后回到框架处理。

protected:
	ExecQueue *get_queue() const { return this->queue; }        // get_queue()：获取当前会话所属队列。

private:
	ExecQueue *queue;                                          // queue：当前任务所属执行队列。

public:
	virtual ~ExecSession() { }                                 // 虚析构函数。
	friend class Executor;                                     // Executor 需要调用 private execute/handle 并设置 queue。
};

class Executor                                                // Executor：线程池执行器。
{
public:
	int init(size_t nthreads);                                 // init(nthreads)：初始化线程池线程数量。
	void deinit();                                             // deinit()：销毁线程池。

	int request(ExecSession *session, ExecQueue *queue);        // request(session,queue)：提交一个计算会话到指定队列。

public:
	int increase_thread();                                     // increase_thread()：动态增加一个执行线程。
	int decrease_thread();                                     // decrease_thread()：动态减少一个执行线程。

private:
	struct __thrdpool *thrdpool;                               // thrdpool：底层 C 风格线程池对象。

private:
	static void executor_thread_routine(void *context);         // executor_thread_routine(context)：线程池工作函数。
	static void executor_cancel(const struct thrdpool_task *task); // executor_cancel(task)：任务取消回调。

public:
	virtual ~Executor() { }                                    // 虚析构函数。
};

#endif                                                        // 结束头文件保护宏。
