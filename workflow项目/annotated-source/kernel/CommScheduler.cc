/*
  注释版源码文件：CommScheduler.cc

  原始文件位置：
  workflow-master/src/kernel/CommScheduler.cc

  本文件用途：
  CommScheduler 是 Workflow 的连接调度层实现。

  核心问题：
  - 一个目标最多允许多少并发连接？
  - 如果连接数满了，请求是等待、立刻失败，还是超时失败？
  - 多个 target 组成 group 时，如何选择负载较低的 target？

  对随机图库项目的意义：
  Redis/MySQL 都会通过 EndpointParams::max_connections 控制到同一个服务的最大连接数。
  这正是面试里讲“高并发但不无限建连接”的关键证据。
*/

#include <sys/types.h>                                         // 系统基础类型。
#include <sys/socket.h>                                        // socket 地址相关类型。
#include <errno.h>                                             // errno、EINVAL、EAGAIN、EBUSY 等错误码。
#include <stdlib.h>                                            // malloc、realloc、free。
#include <pthread.h>                                           // pthread mutex/cond。
#include "CommScheduler.h"                                    // CommSchedTarget、CommSchedGroup 定义。

#define PTHREAD_COND_TIMEDWAIT(cond, mutex, abstime) \          // 等待条件变量的统一宏。
	((abstime) ? pthread_cond_timedwait(cond, mutex, abstime) : \ // 如果 abstime 非空，走带超时等待。
				 pthread_cond_wait(cond, mutex))                // 如果 abstime 为空，走无限等待。

static struct timespec *__get_abstime(int timeout, struct timespec *ts) // __get_abstime：把毫秒超时转换成绝对时间。
{
	if (timeout < 0)                                            // timeout < 0 表示无限等待。
		return NULL;                                            // 返回 NULL，让宏走 pthread_cond_wait。

	clock_gettime(CLOCK_REALTIME, ts);                          // 获取当前实时时钟。
	ts->tv_sec += timeout / 1000;                               // 秒部分加 timeout 的整秒数。
	ts->tv_nsec += timeout % 1000 * 1000000;                    // 纳秒部分加剩余毫秒。
	if (ts->tv_nsec >= 1000000000)                              // 如果纳秒超过 1 秒。
	{
		ts->tv_nsec -= 1000000000;                              // 纳秒减去 1 秒。
		ts->tv_sec++;                                           // 秒数进位。
	}

	return ts;                                                  // 返回绝对超时时间。
}

int CommSchedTarget::init(const struct sockaddr *addr, socklen_t addrlen, // addr/addrlen：目标服务地址。
						  int connect_timeout, int response_timeout,      // connect/response：连接和响应超时。
						  size_t max_connections)                         // max_connections：该目标最大并发连接数。
{
	int ret;                                                    // ret：保存 pthread 初始化返回值。

	if (max_connections == 0)                                   // 最大连接数不能为 0。
	{
		errno = EINVAL;                                         // 设置参数无效。
		return -1;                                             // 初始化失败。
	}

	if (this->CommTarget::init(addr, addrlen, connect_timeout,  // 先初始化父类 CommTarget，包括地址、超时、idle 连接链表。
							   response_timeout) >= 0)
	{
		ret = pthread_mutex_init(&this->mutex, NULL);            // 初始化 target 自己的互斥锁。
		if (ret == 0)                                           // mutex 初始化成功。
		{
			ret = pthread_cond_init(&this->cond, NULL);          // 初始化条件变量，用于连接数满时等待。
			if (ret == 0)                                       // cond 初始化成功。
			{
				this->max_load = max_connections;               // max_load：最大负载，对应最大连接数。
				this->cur_load = 0;                             // cur_load：当前已占用连接/请求数。
				this->wait_cnt = 0;                             // wait_cnt：正在等待该 target 的请求数量。
				this->group = NULL;                             // group：初始不属于任何调度组。
				return 0;                                       // 初始化成功。
			}

			pthread_mutex_destroy(&this->mutex);                 // cond 初始化失败时销毁 mutex。
		}

		errno = ret;                                            // pthread 错误码写入 errno。
		this->CommTarget::deinit();                             // 清理父类资源。
	}

	return -1;                                                  // 初始化失败。
}

void CommSchedTarget::deinit()                                  // deinit()：释放 target 调度资源。
{
	pthread_cond_destroy(&this->cond);                           // 销毁条件变量。
	pthread_mutex_destroy(&this->mutex);                         // 销毁互斥锁。
	this->CommTarget::deinit();                                  // 释放父类 CommTarget 资源。
}

CommTarget *CommSchedTarget::acquire(int wait_timeout)          // acquire(wait_timeout)：申请使用该 target。
{
	pthread_mutex_t *mutex = &this->mutex;                       // mutex：默认使用 target 自己的锁。
	int ret;                                                     // ret：等待或申请结果。

	pthread_mutex_lock(mutex);                                  // 加 target 锁。
	if (this->group)                                            // 如果 target 已经加入调度组。
	{
		mutex = &this->group->mutex;                            // 后续应使用 group 锁保护组内负载。
		pthread_mutex_lock(mutex);                              // 加 group 锁。
		pthread_mutex_unlock(&this->mutex);                      // 释放 target 锁，避免锁顺序问题。
	}

	if (this->cur_load >= this->max_load)                        // 如果当前负载已经达到最大值。
	{
		if (wait_timeout != 0)                                  // wait_timeout != 0 表示允许等待。
		{
			struct timespec ts;                                  // ts：绝对超时时间存储。
			struct timespec *abstime = __get_abstime(wait_timeout, &ts); // 根据 wait_timeout 得到等待时间。

			do                                                  // 循环等待，防止虚假唤醒。
			{
				this->wait_cnt++;                               // 等待者数量加一。
				ret = PTHREAD_COND_TIMEDWAIT(&this->cond, mutex, abstime); // 等待连接释放。
				this->wait_cnt--;                               // 被唤醒或超时后等待者数量减一。
			} while (this->cur_load >= this->max_load && ret == 0); // 如果仍满且不是超时，继续等。
		}
		else
			ret = EAGAIN;                                       // wait_timeout == 0 表示不等，直接 EAGAIN。
	}

	if (this->cur_load < this->max_load)                         // 如果现在有空余负载。
	{
		this->cur_load++;                                       // 当前 target 负载加一。
		if (this->group)                                        // 如果属于 group。
		{
			this->group->cur_load++;                            // group 总负载也加一。
			this->group->heapify(this->index);                  // 当前 target 负载变化，调整堆位置。
		}

		ret = 0;                                                // 申请成功。
	}

	pthread_mutex_unlock(mutex);                                // 解锁。
	if (ret)                                                    // 如果申请失败或等待超时。
	{
		errno = ret;                                            // 把 pthread/调度错误写入 errno。
		return NULL;                                            // 返回 NULL 表示未获得 target。
	}

	return this;                                                // 返回当前 target，调用方可以提交网络请求。
}

void CommSchedTarget::release()                                 // release()：释放一次 target 占用。
{
	pthread_mutex_lock(&this->mutex);                            // 加 target 锁。
	if (this->group)                                            // 如果属于 group。
		pthread_mutex_lock(&this->group->mutex);                 // 同时加 group 锁。

	this->cur_load--;                                           // 当前 target 负载减一。
	if (this->wait_cnt > 0)                                     // 如果有人正等这个 target。
		pthread_cond_signal(&this->cond);                       // 唤醒一个等待者。

	if (this->group)                                            // 如果属于 group，还要维护 group 状态。
	{
		this->group->cur_load--;                                // group 总负载减一。
		if (this->wait_cnt == 0 && this->group->wait_cnt > 0)    // 如果 target 没有专属等待者，但 group 有等待者。
			pthread_cond_signal(&this->group->cond);             // 唤醒一个 group 等待者。

		this->group->heap_adjust(this->index, this->has_idle_conn()); // 负载变低或有 idle 连接时，向堆顶调整。
		pthread_mutex_unlock(&this->group->mutex);               // 释放 group 锁。
	}

	pthread_mutex_unlock(&this->mutex);                          // 释放 target 锁。
}

int CommSchedGroup::target_cmp(CommSchedTarget *target1,        // target1：第一个目标。
							   CommSchedTarget *target2)        // target2：第二个目标。
{
	size_t load1 = target1->cur_load * target2->max_load;        // load1：target1 的归一化负载分子。
	size_t load2 = target2->cur_load * target1->max_load;        // load2：target2 的归一化负载分子。

	if (load1 < load2)                                          // target1 负载比例更低。
		return -1;                                              // target1 优先。
	else if (load1 > load2)                                     // target1 负载比例更高。
		return 1;                                               // target2 优先。
	else
		return 0;                                               // 负载比例相等。
}

void CommSchedGroup::heap_adjust(int index, int swap_on_equal)  // heap_adjust：从 index 向上调整小根堆。
{
	CommSchedTarget *target = this->tg_heap[index];              // target：待调整节点。
	CommSchedTarget *parent;                                     // parent：父节点。

	while (index > 0)                                            // 只要不是根节点。
	{
		parent = this->tg_heap[(index - 1) / 2];                 // 取父节点。
		if (CommSchedGroup::target_cmp(target, parent) < swap_on_equal) // 如果 target 应该排在 parent 前面。
		{
			this->tg_heap[index] = parent;                       // 父节点下移。
			parent->index = index;                               // 更新父节点 index。
			index = (index - 1) / 2;                             // 当前 index 上移到父位置。
		}
		else
			break;                                              // 堆性质已经满足。
	}

	this->tg_heap[index] = target;                               // 把 target 放到最终位置。
	target->index = index;                                       // 更新 target index。
}

void CommSchedGroup::heapify(int top)                           // heapify(top)：从 top 向下调整小根堆。
{
	CommSchedTarget *target = this->tg_heap[top];                // target：待下沉节点。
	int last = this->heap_size - 1;                              // last：最后一个节点下标。
	CommSchedTarget **child;                                     // child：子节点数组指针。
	int i;                                                       // i：左子节点下标。

	while (i = 2 * top + 1, i < last)                            // 当左右子节点都存在。
	{
		child = &this->tg_heap[i];                               // child[0] 左子，child[1] 右子。
		if (CommSchedGroup::target_cmp(child[0], target) < 0)    // 左子比 target 负载更低。
		{
			if (CommSchedGroup::target_cmp(child[1], child[0]) < 0) // 右子比左子更低。
			{
				this->tg_heap[top] = child[1];                   // 右子上移。
				child[1]->index = top;                           // 更新右子 index。
				top = i + 1;                                     // target 准备放到右子位置。
			}
			else                                                 // 左子最低。
			{
				this->tg_heap[top] = child[0];                   // 左子上移。
				child[0]->index = top;                           // 更新左子 index。
				top = i;                                         // target 准备放到左子位置。
			}
		}
		else                                                     // 左子不比 target 低。
		{
			if (CommSchedGroup::target_cmp(child[1], target) < 0) // 但右子比 target 低。
			{
				this->tg_heap[top] = child[1];                   // 右子上移。
				child[1]->index = top;                           // 更新右子 index。
				top = i + 1;                                     // target 下沉到右子位置。
			}
			else                                                 // 两个子节点都不比 target 低。
			{
				this->tg_heap[top] = target;                     // target 放在当前位置。
				target->index = top;                             // 更新 index。
				return;                                          // 调整结束。
			}
		}
	}

	if (i == last)                                               // 只有左子节点存在。
	{
		child = &this->tg_heap[i];                               // child[0] 是最后一个左子。
		if (CommSchedGroup::target_cmp(child[0], target) < 0)    // 如果左子比 target 更低。
		{
			this->tg_heap[top] = child[0];                       // 左子上移。
			child[0]->index = top;                               // 更新左子 index。
			top = i;                                             // target 下沉到左子位置。
		}
	}

	this->tg_heap[top] = target;                                 // 放置 target。
	target->index = top;                                         // 更新 target index。
}

int CommSchedGroup::heap_insert(CommSchedTarget *target)        // heap_insert(target)：把 target 加入堆。
{
	if (this->heap_size == this->heap_buf_size)                  // 如果堆数组容量满了。
	{
		int new_size = 2 * this->heap_buf_size;                  // 新容量扩大为 2 倍。
		void *new_base = realloc(this->tg_heap, new_size * sizeof (void *)); // 重新分配数组。

		if (new_base)                                           // 扩容成功。
		{
			this->tg_heap = (CommSchedTarget **)new_base;        // 更新数组指针。
			this->heap_buf_size = new_size;                      // 更新容量。
		}
		else
			return -1;                                          // 扩容失败。
	}

	this->tg_heap[this->heap_size] = target;                     // 把新 target 放到堆尾。
	target->index = this->heap_size;                             // 记录 target 当前下标。
	this->heap_adjust(this->heap_size, 0);                       // 向上调整，恢复小根堆。
	this->heap_size++;                                          // 堆元素数量加一。
	return 0;                                                   // 插入成功。
}

void CommSchedGroup::heap_remove(int index)                     // heap_remove(index)：删除堆中指定下标的 target。
{
	CommSchedTarget *target;                                     // target：从堆尾拿来填补的节点。

	this->heap_size--;                                          // 堆大小先减一。
	if (index != this->heap_size)                               // 如果删除的不是最后一个节点。
	{
		target = this->tg_heap[this->heap_size];                 // 取最后一个节点填补空洞。
		this->tg_heap[index] = target;                           // 放到被删除的位置。
		target->index = index;                                   // 更新 index。
		this->heap_adjust(index, 0);                             // 可能需要向上调整。
		this->heapify(target->index);                            // 也可能需要向下调整。
	}
}

#define COMMGROUP_INIT_SIZE		4                              // group 堆初始容量。

int CommSchedGroup::init()                                      // init()：初始化调度组。
{
	size_t size = COMMGROUP_INIT_SIZE * sizeof (void *);         // 初始堆数组字节数。
	int ret;                                                     // ret：pthread 返回值。

	this->tg_heap = (CommSchedTarget **)malloc(size);            // 分配 target 堆数组。
	if (this->tg_heap)                                          // 分配成功。
	{
		ret = pthread_mutex_init(&this->mutex, NULL);            // 初始化 group 锁。
		if (ret == 0)                                           // mutex 成功。
		{
			ret = pthread_cond_init(&this->cond, NULL);          // 初始化 group 条件变量。
			if (ret == 0)                                       // cond 成功。
			{
				this->heap_buf_size = COMMGROUP_INIT_SIZE;       // 设置堆容量。
				this->heap_size = 0;                             // 当前没有 target。
				this->max_load = 0;                              // group 最大负载为 0。
				this->cur_load = 0;                              // group 当前负载为 0。
				this->wait_cnt = 0;                              // group 等待者为 0。
				return 0;                                       // 初始化成功。
			}

			pthread_mutex_destroy(&this->mutex);                 // cond 失败时销毁 mutex。
		}

		errno = ret;                                            // 记录 pthread 错误。
		free(this->tg_heap);                                    // 释放堆数组。
	}

	return -1;                                                  // 初始化失败。
}

void CommSchedGroup::deinit()                                   // deinit()：释放调度组资源。
{
	pthread_cond_destroy(&this->cond);                           // 销毁条件变量。
	pthread_mutex_destroy(&this->mutex);                         // 销毁互斥锁。
	free(this->tg_heap);                                        // 释放堆数组。
}

int CommSchedGroup::add(CommSchedTarget *target)                // add(target)：把 target 加入 group。
{
	int ret = -1;                                                // ret：默认失败。

	pthread_mutex_lock(&target->mutex);                          // 先锁 target。
	pthread_mutex_lock(&this->mutex);                            // 再锁 group。
	if (target->group == NULL && target->wait_cnt == 0)          // target 没有归属 group，且没有线程正在等它。
	{
		if (this->heap_insert(target) >= 0)                      // 插入堆成功。
		{
			target->group = this;                                // 标记 target 所属 group。
			this->max_load += target->max_load;                  // group 最大负载增加。
			this->cur_load += target->cur_load;                  // group 当前负载增加。
			if (this->wait_cnt > 0 && this->cur_load < this->max_load) // 如果 group 有等待者且现在有空位。
				pthread_cond_signal(&this->cond);                // 唤醒一个等待者。

			ret = 0;                                            // 添加成功。
		}
	}
	else if (target->group == this)                              // target 已经在当前 group。
		errno = EEXIST;                                         // 设置已存在。
	else if (target->group)                                      // target 在其他 group。
		errno = EINVAL;                                         // 参数状态不合法。
	else
		errno = EBUSY;                                          // target 有等待者，暂时忙。

	pthread_mutex_unlock(&this->mutex);                          // 解 group 锁。
	pthread_mutex_unlock(&target->mutex);                        // 解 target 锁。
	return ret;                                                 // 返回结果。
}

int CommSchedGroup::remove(CommSchedTarget *target)             // remove(target)：从 group 移除 target。
{
	int ret = -1;                                                // ret：默认失败。

	pthread_mutex_lock(&target->mutex);                          // 先锁 target。
	pthread_mutex_lock(&this->mutex);                            // 再锁 group。
	if (target->group == this && target->wait_cnt == 0)          // target 属于当前 group，且没有等待者。
	{
		this->heap_remove(target->index);                        // 从堆中删除。
		this->max_load -= target->max_load;                      // 减少 group 最大负载。
		this->cur_load -= target->cur_load;                      // 减少 group 当前负载。
		target->group = NULL;                                    // 清空 target group。
		ret = 0;                                                // 删除成功。
	}
	else if (target->group != this)                              // target 不属于当前 group。
		errno = ENOENT;                                         // 设置不存在。
	else
		errno = EBUSY;                                          // 有等待者，不能删除。

	pthread_mutex_unlock(&this->mutex);                          // 解 group 锁。
	pthread_mutex_unlock(&target->mutex);                        // 解 target 锁。
	return ret;                                                 // 返回结果。
}

CommTarget *CommSchedGroup::acquire(int wait_timeout)           // acquire(wait_timeout)：从 group 中选择一个 target。
{
	pthread_mutex_t *mutex = &this->mutex;                       // group 操作使用 group 锁。
	CommSchedTarget *target;                                     // target：最终选中的目标。
	int ret;                                                     // ret：等待/申请结果。

	pthread_mutex_lock(mutex);                                  // 加 group 锁。
	if (this->cur_load >= this->max_load)                        // 如果 group 整体负载已满。
	{
		if (wait_timeout != 0)                                  // 如果允许等待。
		{
			struct timespec ts;                                  // ts：绝对超时时间。
			struct timespec *abstime = __get_abstime(wait_timeout, &ts); // 转换 timeout。

			do                                                  // 循环等待，防止虚假唤醒。
			{
				this->wait_cnt++;                               // group 等待者加一。
				ret = PTHREAD_COND_TIMEDWAIT(&this->cond, mutex, abstime); // 等待 group 有空负载。
				this->wait_cnt--;                               // 等待结束，数量减一。
			} while (this->cur_load >= this->max_load && ret == 0); // 仍满则继续等。
		}
		else
			ret = EAGAIN;                                       // 不等待，直接返回 EAGAIN。
	}

	if (this->cur_load < this->max_load)                         // 如果 group 有空余负载。
	{
		target = this->tg_heap[0];                               // 小根堆堆顶是负载比例最低的 target。
		target->cur_load++;                                     // 选中 target 负载加一。
		this->cur_load++;                                       // group 总负载加一。
		this->heapify(0);                                       // 堆顶负载变化，向下调整。
		ret = 0;                                                // 申请成功。
	}

	pthread_mutex_unlock(mutex);                                // 解锁。
	if (ret)                                                    // 如果申请失败。
	{
		errno = ret;                                            // 设置 errno。
		return NULL;                                            // 返回 NULL。
	}

	return target;                                              // 返回被选中的 target。
}

