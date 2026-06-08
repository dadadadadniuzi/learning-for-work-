/*
  注释版源码文件：mpoller.h

  原始文件位置：
  workflow-master/src/kernel/mpoller.h

  本文件用途：
  mpoller 是 multi-poller，即多个 poller 的封装。

  面试理解：
  1. 单个 poller 可以理解为一个事件循环。
  2. mpoller 内部有多个 poller，通常对应多个 poller 线程。
  3. fd 会按 fd % nthreads 分配到某一个 poller，分散事件监听压力。
*/

#ifndef _MPOLLER_H_                                           // 头文件保护宏。
#define _MPOLLER_H_                                           // 定义头文件保护宏。

#include <stddef.h>                                           // size_t。
#include "poller.h"                                           // poller_t、poller_data、poller_result。

typedef struct __mpoller mpoller_t;                           // mpoller_t：多 poller 对象。

#ifdef __cplusplus
extern "C"                                                    // C++ 下使用 C 链接。
{
#endif

mpoller_t *mpoller_create(const struct poller_params *params, size_t nthreads); // 创建包含 nthreads 个 poller 的 mpoller。
int mpoller_start(mpoller_t *mpoller);                         // 启动所有 poller。
void mpoller_set_callback(void (*callback)(struct poller_result *, void *),
						  mpoller_t *mpoller);                 // 设置所有 poller 的回调。
void mpoller_stop(mpoller_t *mpoller);                         // 停止所有 poller。
void mpoller_destroy(mpoller_t *mpoller);                      // 销毁 mpoller。

#ifdef __cplusplus
}
#endif

struct __mpoller                                              // __mpoller：多 poller 内部结构。
{
	void **nodes_buf;                                          // nodes_buf：内部节点缓冲区，具体实现中使用。
	unsigned int nthreads;                                     // nthreads：poller 数量，也就是 poller 线程数量。
	poller_t *poller[1];                                       // poller：柔性数组风格，保存多个 poller 指针。
};

static inline int mpoller_add(const struct poller_data *data, int timeout,
							  mpoller_t *mpoller)              // mpoller_add()：添加 fd 事件到某个 poller。
{
	int index = (unsigned int)data->fd % mpoller->nthreads;     // index：按 fd 取模选择 poller，分散不同 fd。
	return poller_add(data, timeout, mpoller->poller[index]);   // 把事件交给对应 poller。
}

static inline int mpoller_del(int fd, mpoller_t *mpoller)      // mpoller_del()：删除 fd 事件。
{
	int index = (unsigned int)fd % mpoller->nthreads;           // 用同样的 fd % nthreads 找回所属 poller。
	return poller_del(fd, mpoller->poller[index]);              // 从对应 poller 删除。
}

static inline int mpoller_mod(const struct poller_data *data, int timeout,
							  mpoller_t *mpoller)              // mpoller_mod()：修改 fd 事件。
{
	int index = (unsigned int)data->fd % mpoller->nthreads;     // 找到 fd 所属 poller。
	return poller_mod(data, timeout, mpoller->poller[index]);   // 修改对应 poller 中的事件。
}

static inline int mpoller_set_timeout(int fd, int timeout, mpoller_t *mpoller) // 设置 fd 超时。
{
	int index = (unsigned int)fd % mpoller->nthreads;           // 找到 fd 所属 poller。
	return poller_set_timeout(fd, timeout, mpoller->poller[index]); // 修改超时。
}

static inline int mpoller_add_timer(const struct timespec *value, void *context,
									void **timer, int *index,
									mpoller_t *mpoller)          // 添加定时器。
{
	static unsigned int n = 0;                                  // n：静态轮询计数器，用于把 timer 均匀分到不同 poller。
	*index = n++ % mpoller->nthreads;                          // 输出 timer 所属 poller 下标，删除时要用。
	return poller_add_timer(value, context, timer, mpoller->poller[*index]); // 添加到选中的 poller。
}

static inline int mpoller_del_timer(void *timer, int index, mpoller_t *mpoller) // 删除定时器。
{
	return poller_del_timer(timer, mpoller->poller[index]);     // 按保存的 index 找到原 poller 删除 timer。
}

#endif                                                        // 结束头文件保护宏。
