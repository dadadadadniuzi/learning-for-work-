/*
  注释版源码文件：thrdpool.c

  原始文件位置：
  workflow-master/src/kernel/thrdpool.c

  本文件用途：
  Workflow 底层 C 线程池实现。

  面试理解：
  1. thrdpool 内部有一个 msgqueue，用来存放待执行任务。
  2. 每个工作线程循环从 msgqueue_get() 取任务。
  3. 取到任务后执行 task.routine(task.context)。
  4. Executor 和 Communicator 的 handler 线程池都依赖类似机制。
*/

#include <errno.h>                                            // errno。
#include <stdlib.h>                                           // malloc/free。
#include <pthread.h>                                          // pthread。
#include "msgqueue.h"                                         // 消息队列。
#include "thrdpool.h"                                         // 线程池类型和任务结构声明。

struct __thrdpool                                             // __thrdpool：线程池内部结构。
{
	msgqueue_t *msgqueue;                                      // msgqueue：任务队列，工作线程从这里取任务。
	size_t nthreads;                                           // nthreads：当前线程数量。
	size_t stacksize;                                          // stacksize：线程栈大小，0 表示默认。
	pthread_t tid;                                             // tid：用于线程退出时串联 join 的线程 id。
	pthread_mutex_t mutex;                                     // mutex：保护 nthreads/tid/terminate。
	pthread_key_t key;                                         // key：线程局部存储，用于判断当前线程是否属于该线程池。
	pthread_cond_t *terminate;                                 // terminate：销毁线程池时用于等待所有线程退出。
};

struct __thrdpool_task_entry                                  // __thrdpool_task_entry：队列中的任务节点。
{
	void *link;                                                // link：msgqueue 内部链表指针。
	struct thrdpool_task task;                                 // task：真正的任务函数和上下文。
};

static pthread_t __zero_tid;                                  // __zero_tid：全 0 线程 id，用于判断 tid 是否有效。

static void __thrdpool_exit_routine(void *context)             // __thrdpool_exit_routine(context)：工作线程退出逻辑。
{
	thrdpool_t *pool = (thrdpool_t *)context;                  // pool：当前线程池。
	pthread_t tid;                                             // tid：需要 join 的上一个线程 id。

	pthread_mutex_lock(&pool->mutex);                          // 加锁保护 nthreads/tid。
	tid = pool->tid;                                           // 保存旧 tid，用于稍后 join。
	pool->tid = pthread_self();                                // 当前退出线程把自己的 tid 存进去，形成链式 join。
	if (--pool->nthreads == 0 && pool->terminate)              // 当前线程退出，线程数减 1；如果已经全部退出且有人等待销毁。
		pthread_cond_signal(pool->terminate);                  // 通知销毁线程可以继续。

	pthread_mutex_unlock(&pool->mutex);                        // 解锁。
	if (!pthread_equal(tid, __zero_tid))                       // 如果旧 tid 有效。
		pthread_join(tid, NULL);                               // join 旧线程，避免资源泄漏。

	pthread_exit(NULL);                                        // 当前线程退出。
}

static void *__thrdpool_routine(void *arg)                     // __thrdpool_routine(arg)：每个工作线程的主循环。
{
	thrdpool_t *pool = (thrdpool_t *)arg;                      // pool：线程池对象。
	struct __thrdpool_task_entry *entry;                       // entry：从队列取出的任务节点。
	void (*task_routine)(void *);                              // task_routine：任务函数。
	void *task_context;                                        // task_context：任务上下文。

	pthread_setspecific(pool->key, pool);                      // 设置线程局部变量，标记当前线程属于该 pool。
	while (!pool->terminate)                                   // 只要线程池没有进入销毁状态。
	{
		entry = (struct __thrdpool_task_entry *)msgqueue_get(pool->msgqueue); // 阻塞/非阻塞获取任务。
		if (!entry)                                            // 如果队列关闭或没有任务。
			break;                                             // 退出循环。

		task_routine = entry->task.routine;                    // 取出任务函数。
		task_context = entry->task.context;                    // 取出任务上下文。
		free(entry);                                           // 任务节点可以释放，函数和上下文已保存。
		task_routine(task_context);                            // 执行任务。

		if (pool->nthreads == 0)                               // 如果任务内部销毁了线程池。
		{
			free(pool);                                        // 释放 pool 对象。
			return NULL;                                       // 直接返回。
		}
	}

	__thrdpool_exit_routine(pool);                             // 正常退出线程。
	return NULL;                                               // 理论上不会走到这里，因为 exit_routine 调 pthread_exit。
}

static void __thrdpool_terminate(int in_pool, thrdpool_t *pool) // __thrdpool_terminate()：终止线程池所有线程。
{
	pthread_cond_t term = PTHREAD_COND_INITIALIZER;             // term：临时条件变量，用于等待线程退出。

	pthread_mutex_lock(&pool->mutex);                          // 加锁。
	msgqueue_set_nonblock(pool->msgqueue);                     // 把队列设为非阻塞，让等待中的线程醒来退出。
	pool->terminate = &term;                                   // 设置 terminate 指针，通知线程池正在销毁。

	if (in_pool)                                               // 如果销毁操作发生在线程池内部线程。
	{
		pthread_detach(pthread_self());                        // 当前线程不能 join 自己，先 detach。
		pool->nthreads--;                                      // 当前线程视为已经退出。
	}

	while (pool->nthreads > 0)                                 // 等待其他线程全部退出。
		pthread_cond_wait(&term, &pool->mutex);                // 条件变量等待。

	pthread_mutex_unlock(&pool->mutex);                        // 解锁。
	if (!pthread_equal(pool->tid, __zero_tid))                 // 如果还有链式保存的线程 id。
		pthread_join(pool->tid, NULL);                         // join 它。

	pthread_cond_destroy(&term);                               // 销毁条件变量。
}

static int __thrdpool_create_threads(size_t nthreads, thrdpool_t *pool) // 创建 nthreads 个工作线程。
{
	pthread_attr_t attr;                                       // attr：线程属性。
	pthread_t tid;                                             // tid：新线程 id。
	int ret;                                                   // ret：pthread 调用返回值。

	ret = pthread_attr_init(&attr);                            // 初始化线程属性。
	if (ret == 0)
	{
		if (pool->stacksize)                                   // 如果用户指定栈大小。
			ret = pthread_attr_setstacksize(&attr, pool->stacksize); // 设置线程栈大小。

		if (ret == 0)
		{
			pthread_mutex_lock(&pool->mutex);                  // 加锁保护 nthreads。
			while (pool->nthreads < nthreads)                  // 循环创建直到达到目标数量。
			{
				ret = pthread_create(&tid, &attr, __thrdpool_routine, pool); // 创建工作线程。
				if (ret == 0)
					pool->nthreads++;                         // 创建成功，线程数加 1。
				else
					break;                                    // 创建失败，停止。
			}

			pthread_mutex_unlock(&pool->mutex);                // 解锁。
		}

		pthread_attr_destroy(&attr);                           // 销毁线程属性。
		if (ret == 0)
			return 0;                                         // 全部创建成功。

		__thrdpool_terminate(0, pool);                         // 创建中途失败，终止已创建线程。
	}

	errno = ret;                                               // pthread 错误码写入 errno。
	return -1;                                                 // 返回失败。
}

thrdpool_t *thrdpool_create(size_t nthreads, size_t stacksize) // thrdpool_create()：创建线程池。
{
	thrdpool_t *pool;                                          // pool：线程池对象。
	int ret;                                                   // ret：错误码。

	pool = (thrdpool_t *)malloc(sizeof (thrdpool_t));          // 分配线程池结构。
	if (!pool)
		return NULL;                                          // 分配失败。

	pool->msgqueue = msgqueue_create(0, 0);                    // 创建任务消息队列。
	if (pool->msgqueue)
	{
		ret = pthread_mutex_init(&pool->mutex, NULL);           // 初始化互斥锁。
		if (ret == 0)
		{
			ret = pthread_key_create(&pool->key, NULL);         // 创建线程局部 key。
			if (ret == 0)
			{
				pool->stacksize = stacksize;                   // 保存栈大小。
				pool->nthreads = 0;                            // 初始线程数 0。
				pool->tid = __zero_tid;                        // 初始没有待 join 线程。
				pool->terminate = NULL;                        // 初始不处于销毁状态。
				if (__thrdpool_create_threads(nthreads, pool) >= 0) // 创建工作线程。
					return pool;                               // 成功返回线程池。

				pthread_key_delete(pool->key);                 // 创建失败，删除 key。
			}

			pthread_mutex_destroy(&pool->mutex);               // 销毁锁。
		}

		errno = ret;                                           // 保存错误码。
		msgqueue_destroy(pool->msgqueue);                      // 销毁队列。
	}

	free(pool);                                                // 释放 pool。
	return NULL;                                               // 返回失败。
}

void __thrdpool_schedule(const struct thrdpool_task *task, void *buf,
						 thrdpool_t *pool)                    // 内部调度函数，buf 由调用者提供。
{
	((struct __thrdpool_task_entry *)buf)->task = *task;       // 把任务复制进队列节点。
	msgqueue_put(buf, pool->msgqueue);                         // 放入消息队列，等待工作线程取走。
}

int thrdpool_schedule(const struct thrdpool_task *task, thrdpool_t *pool) // 提交任务到线程池。
{
	void *buf = malloc(sizeof (struct __thrdpool_task_entry)); // 分配任务节点。

	if (buf)
	{
		__thrdpool_schedule(task, buf, pool);                  // 放入队列。
		return 0;                                             // 成功。
	}

	return -1;                                                // 分配失败。
}

int thrdpool_in_pool(thrdpool_t *pool)                         // 判断当前线程是否属于 pool。
{
	return pthread_getspecific(pool->key) == pool;              // 比较线程局部变量。
}

int thrdpool_increase(thrdpool_t *pool)                        // 增加一个工作线程。
{
	pthread_attr_t attr;                                       // 线程属性。
	pthread_t tid;                                             // 新线程 id。
	int ret;                                                   // 返回值。

	ret = pthread_attr_init(&attr);                            // 初始化属性。
	if (ret == 0)
	{
		if (pool->stacksize)
			ret = pthread_attr_setstacksize(&attr, pool->stacksize); // 设置栈大小。

		if (ret == 0)
		{
			pthread_mutex_lock(&pool->mutex);                  // 加锁。
			ret = pthread_create(&tid, &attr, __thrdpool_routine, pool); // 创建线程。
			if (ret == 0)
				pool->nthreads++;                             // 成功则线程数加 1。

			pthread_mutex_unlock(&pool->mutex);                // 解锁。
		}

		pthread_attr_destroy(&attr);                           // 销毁属性。
		if (ret == 0)
			return 0;                                         // 成功。
	}

	errno = ret;                                               // 保存 pthread 错误。
	return -1;                                                 // 失败。
}

int thrdpool_decrease(thrdpool_t *pool)                        // 减少一个工作线程。
{
	void *buf = malloc(sizeof (struct __thrdpool_task_entry)); // 分配一个特殊任务节点。
	struct __thrdpool_task_entry *entry;                       // entry：任务节点。

	if (buf)
	{
		entry = (struct __thrdpool_task_entry *)buf;            // 转换类型。
		entry->task.routine = __thrdpool_exit_routine;         // 特殊任务函数：让执行它的线程退出。
		entry->task.context = pool;                            // 上下文是 pool。
		msgqueue_put_head(entry, pool->msgqueue);              // 放到队头，尽快被执行。
		return 0;                                             // 成功。
	}

	return -1;                                                // 分配失败。
}

void thrdpool_exit(thrdpool_t *pool)                           // 当前工作线程主动退出。
{
	if (thrdpool_in_pool(pool))                                // 只有线程池内部线程才能调用。
		__thrdpool_exit_routine(pool);                         // 执行退出逻辑。
}

void thrdpool_destroy(void (*pending)(const struct thrdpool_task *),
					  thrdpool_t *pool)                       // 销毁线程池。
{
	int in_pool = thrdpool_in_pool(pool);                       // in_pool：当前是否在线程池内部调用 destroy。
	struct __thrdpool_task_entry *entry;                        // entry：未执行任务节点。

	__thrdpool_terminate(in_pool, pool);                        // 终止所有工作线程。
	while (1)
	{
		entry = (struct __thrdpool_task_entry *)msgqueue_get(pool->msgqueue); // 取出剩余未执行任务。
		if (!entry)
			break;                                             // 没有剩余任务。

		if (pending && entry->task.routine != __thrdpool_exit_routine) // 如果有 pending 回调且不是退出任务。
			pending(&entry->task);                             // 通知调用者这个任务未执行。

		free(entry);                                           // 释放任务节点。
	}

	pthread_key_delete(pool->key);                             // 删除线程局部 key。
	pthread_mutex_destroy(&pool->mutex);                       // 销毁锁。
	msgqueue_destroy(pool->msgqueue);                          // 销毁任务队列。
	if (!in_pool)                                              // 如果 destroy 不是在线程池内部调用。
		free(pool);                                            // 释放 pool。
}
