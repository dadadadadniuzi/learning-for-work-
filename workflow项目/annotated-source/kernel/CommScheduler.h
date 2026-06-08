/*
  注释版源码文件：CommScheduler.h

  原始文件位置：
  workflow-master/src/kernel/CommScheduler.h

  本文件用途：
  CommScheduler 是 Communicator 上面的一层“连接调度壳”。

  对高并发面试最重要：
  1. Communicator 负责真正的非阻塞 IO。
  2. CommScheduler 负责在发请求前选择目标、控制连接负载、限制最大连接数。
  3. HTTP/Redis/MySQL 客户端任务最终会通过 CommScheduler::request() 发起网络请求。

  一句话：
  Communicator 解决“怎么异步通信”，CommScheduler 解决“请求交给哪个目标、连接怎么控”。
*/

#ifndef _COMMSCHEDULER_H_                                      // 头文件保护宏：防止重复 include。
#define _COMMSCHEDULER_H_                                      // 定义头文件保护宏。

#include <sys/types.h>                                         // 系统类型。
#include <sys/socket.h>                                        // sockaddr/socklen_t。
#include <pthread.h>                                           // pthread_mutex_t / pthread_cond_t。
#include <openssl/ssl.h>                                       // SSL_CTX。
#include "Communicator.h"                                      // Communicator、CommTarget、CommSession 等底层通信类型。

class CommSchedObject                                          // CommSchedObject：可被调度器 acquire 的抽象对象。
{
public:
	size_t get_max_load() const { return this->max_load; }      // get_max_load()：获取最大负载，通常对应最大连接数。
	size_t get_cur_load() const { return this->cur_load; }      // get_cur_load()：获取当前负载，通常对应当前占用连接数。

private:
	virtual CommTarget *acquire(int wait_timeout) = 0;          // acquire(wait_timeout)：获取一个可用通信目标。
	                                                           // wait_timeout：等待可用连接的超时时间，毫秒。
	                                                           // 返回 CommTarget*：成功获得的目标；NULL 表示失败。

protected:
	size_t max_load;                                            // max_load：允许的最大负载。
	size_t cur_load;                                            // cur_load：当前负载。

public:
	virtual ~CommSchedObject() { }                              // 虚析构函数。
	friend class CommScheduler;                                 // CommScheduler 需要调用 private acquire()。
};

class CommSchedGroup;                                          // 前置声明目标组。

class CommSchedTarget : public CommSchedObject, public CommTarget // CommSchedTarget：单个可调度目标。
{
public:
	int init(const struct sockaddr *addr, socklen_t addrlen,    // addr/addrlen：目标地址。
			 int connect_timeout,                              // connect_timeout：连接超时。
			 int response_timeout,                             // response_timeout：单次响应超时。
			 size_t max_connections);                          // max_connections：这个目标最大连接数。
	void deinit();                                             // deinit()：释放目标资源。

public:
	int init(const struct sockaddr *addr, socklen_t addrlen, SSL_CTX *ssl_ctx,
			 int connect_timeout, int ssl_connect_timeout, int response_timeout,
			 size_t max_connections)                           // SSL 版本 init。
	{
		int ret = this->init(addr, addrlen, connect_timeout, response_timeout,
							 max_connections);                 // 先初始化普通目标。

		if (ret >= 0)                                          // 如果普通初始化成功。
			this->set_ssl(ssl_ctx, ssl_connect_timeout);        // 设置 SSL 上下文和 SSL 握手超时。

		return ret;                                            // 返回初始化结果。
	}

private:
	virtual CommTarget *acquire(int wait_timeout);              // acquire()：获取当前目标使用权，内部会检查负载和等待。
	virtual void release();                                     // release()：归还目标使用权，降低负载并唤醒等待者。

private:
	CommSchedGroup *group;                                      // group：当前目标所属目标组；单目标时可为空。
	int index;                                                  // index：目标在 group 堆数组中的下标。
	int wait_cnt;                                               // wait_cnt：等待该目标可用的请求数量。
	pthread_mutex_t mutex;                                      // mutex：保护 cur_load/wait_cnt 等状态。
	pthread_cond_t cond;                                        // cond：连接释放时通知等待线程。
	friend class CommSchedGroup;                                // CommSchedGroup 需要访问内部字段维护堆。
};

class CommSchedGroup : public CommSchedObject                   // CommSchedGroup：多个 CommSchedTarget 组成的调度组。
{
public:
	int init();                                                 // init()：初始化目标组。
	void deinit();                                              // deinit()：释放目标组。
	int add(CommSchedTarget *target);                           // add(target)：加入一个目标。
	int remove(CommSchedTarget *target);                        // remove(target)：移除一个目标。

private:
	virtual CommTarget *acquire(int wait_timeout);              // acquire()：从组里选出一个可用目标。

private:
	CommSchedTarget **tg_heap;                                  // tg_heap：目标最小堆/优先队列，用于选择负载较低目标。
	int heap_size;                                              // heap_size：当前堆中目标数量。
	int heap_buf_size;                                          // heap_buf_size：堆数组容量。
	int wait_cnt;                                               // wait_cnt：等待整个组可用的请求数量。
	pthread_mutex_t mutex;                                      // mutex：保护堆和组负载。
	pthread_cond_t cond;                                        // cond：目标释放时通知等待者。

private:
	static int target_cmp(CommSchedTarget *target1, CommSchedTarget *target2); // 比较两个目标的负载优先级。
	void heapify(int top);                                      // heapify(top)：从 top 向下调整堆。
	void heap_adjust(int index, int swap_on_equal);             // heap_adjust(index)：从 index 调整堆位置。
	int heap_insert(CommSchedTarget *target);                   // heap_insert(target)：插入目标到堆。
	void heap_remove(int index);                                // heap_remove(index)：从堆移除目标。
	friend class CommSchedTarget;                               // 目标 release 时可能需要调整所在组堆。
};

class CommScheduler                                            // CommScheduler：通信调度器，对外提供 request/reply/bind/sleep/io_bind 等统一接口。
{
public:
	int init(size_t poller_threads, size_t handler_threads)     // init(poller_threads, handler_threads)：初始化底层通信器。
	{
		return this->comm.init(poller_threads, handler_threads); // 交给 Communicator 创建 poller 线程和 handler 线程。
	}

	void deinit()                                               // deinit()：销毁底层通信器。
	{
		this->comm.deinit();                                    // 释放 Communicator 资源。
	}

	int request(CommSession *session,                           // session：一次通信会话，比如 HTTP/Redis/MySQL 任务。
				CommSchedObject *object,                       // object：可调度目标或目标组。
				int wait_timeout,                              // wait_timeout：等待连接可用的超时。
				CommTarget **target)                           // target：输出参数，保存 acquire 到的目标。
	{
		int ret = -1;                                           // ret：最终返回值，默认失败。

		*target = object->acquire(wait_timeout);                // 先从调度对象中获取一个可用目标。
		if (*target)                                            // 如果成功拿到目标。
		{
			ret = this->comm.request(session, *target);          // 把通信会话交给 Communicator 发起真正网络请求。
			if (ret < 0)                                        // 如果提交给 Communicator 失败。
				(*target)->release();                           // 归还目标，避免负载计数泄漏。
		}

		return ret;                                             // 返回 0 或负数。
	}

	int reply(CommSession *session)                             // reply(session)：服务端回复客户端。
	{
		return this->comm.reply(session);                        // 交给 Communicator 发送响应。
	}

	int shutdown(CommSession *session)                          // shutdown(session)：关闭会话连接。
	{
		return this->comm.shutdown(session);
	}

	int push(const void *buf, size_t size, CommSession *session) // push(buf, size, session)：向连接直接推送数据。
	{
		return this->comm.push(buf, size, session);
	}

	int bind(CommService *service)                              // bind(service)：绑定服务监听。
	{
		return this->comm.bind(service);                         // 服务端 start 时会走这里。
	}

	void unbind(CommService *service)                           // unbind(service)：取消服务监听。
	{
		this->comm.unbind(service);
	}

	int sleep(SleepSession *session)                            // sleep(session)：注册定时器任务。
	{
		return this->comm.sleep(session);
	}

	int unsleep(SleepSession *session)                          // unsleep(session)：取消定时器。
	{
		return this->comm.unsleep(session);
	}

	int io_bind(IOService *service)                             // io_bind(service)：绑定文件 IO 服务。
	{
		return this->comm.io_bind(service);
	}

	void io_unbind(IOService *service)                          // io_unbind(service)：解绑文件 IO 服务。
	{
		this->comm.io_unbind(service);
	}

public:
	int is_handler_thread() const                               // is_handler_thread()：判断当前线程是否是 handler 线程。
	{
		return this->comm.is_handler_thread();
	}

	int increase_handler_thread()                               // increase_handler_thread()：动态增加 handler 线程。
	{
		return this->comm.increase_handler_thread();
	}

	int decrease_handler_thread()                               // decrease_handler_thread()：动态减少 handler 线程。
	{
		return this->comm.decrease_handler_thread();
	}

public:
	void customize_event_handler(CommEventHandler *handler)     // customize_event_handler(handler)：自定义事件处理器。
	{
		this->comm.customize_event_handler(handler);             // 交给 Communicator 保存。
	}

private:
	Communicator comm;                                          // comm：真正干网络 IO、定时器、文件 IO 的底层通信器。

public:
	virtual ~CommScheduler() { }                                // 虚析构函数。
};

#endif                                                        // 结束头文件保护宏。
