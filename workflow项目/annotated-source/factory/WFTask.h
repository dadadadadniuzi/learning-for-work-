/*
  注释版源码文件：WFTask.h

  原始文件位置：
  workflow-master/src/factory/WFTask.h

  本文件用途：
  定义 Workflow 上层最常见的任务类型。

  对随机图库项目最重要的是：
  1. WFNetworkTask<REQ, RESP>：HTTP、Redis、MySQL、DNS 任务共同基类。
  2. WFTimerTask：定时器任务。
  3. WFFileTask<ARGS>：文件异步 IO 任务，用于读写图片文件。
  4. WFGenericTask 及其派生类：一些非网络、非文件的通用控制任务。

  阅读主线：
  SubTask 只规定“任务怎么启动和完成”；
  WFTask.h 则把网络、线程池、定时器、文件 IO 等具体能力包装成统一任务对象。
*/

#ifndef _WFTASK_H_                                           // 头文件保护宏：避免重复 include。
#define _WFTASK_H_                                           // 定义头文件保护宏。

#include <errno.h>                                           // errno：用于错误码，如 ENOTCONN、ENOENT、ETIMEDOUT。
#include <string.h>                                          // string.h：用于 memcpy 等 C 字符串/内存操作。
#include <assert.h>                                          // assert：用于调试期检查任务状态是否合法。
#include <atomic>                                            // atomic：Counter/Mailbox/Selector 等任务需要原子变量。
#include <utility>                                           // utility：提供 std::move。
#include <functional>                                        // functional：提供 std::function，用于保存回调。
#include "Executor.h"                                        // Executor：计算任务线程池执行器。
#include "ExecRequest.h"                                     // ExecRequest：线程池任务请求基类。
#include "Communicator.h"                                    // Communicator：底层通信会话、消息输入输出等。
#include "CommScheduler.h"                                   // CommScheduler：通信调度器。
#include "CommRequest.h"                                     // CommRequest：网络请求任务基类。
#include "SleepRequest.h"                                    // SleepRequest：定时器任务基类。
#include "IORequest.h"                                       // IORequest：文件 IO 任务基类。
#include "Workflow.h"                                        // Workflow/SeriesWork：任务编排核心。
#include "WFConnection.h"                                    // WFConnection：Workflow 封装的连接对象。

enum                                                          // Workflow 任务统一状态码。
{
	WFT_STATE_UNDEFINED = -1,                                 // UNDEFINED：任务刚创建，还没有运行。
	WFT_STATE_SUCCESS = CS_STATE_SUCCESS,                     // SUCCESS：任务成功完成。
	WFT_STATE_TOREPLY = CS_STATE_TOREPLY,                     // TOREPLY：服务端任务专用，表示请求已处理，准备回复。
	WFT_STATE_NOREPLY = CS_STATE_TOREPLY + 1,                 // NOREPLY：服务端任务专用，表示用户调用 noreply() 不再回复。
	WFT_STATE_SYS_ERROR = CS_STATE_ERROR,                     // SYS_ERROR：系统错误，error 通常是 errno。
	WFT_STATE_SSL_ERROR = 65,                                 // SSL_ERROR：SSL/TLS 相关错误。
	WFT_STATE_DNS_ERROR = 66,                                 // DNS_ERROR：客户端任务专用，表示 DNS 解析失败。
	WFT_STATE_TASK_ERROR = 67,                                // TASK_ERROR：任务级错误，如 URL 非法、协议错误等。
	WFT_STATE_ABORTED = CS_STATE_STOPPED                      // ABORTED：任务被终止，比如进程退出或调度停止。
};

template<class INPUT, class OUTPUT>                          // 模板参数 INPUT：线程任务输入类型；OUTPUT：线程任务输出类型。
class WFThreadTask : public ExecRequest                       // WFThreadTask：把一个线程池计算任务包装成 Workflow 任务。
{
public:
	void start()                                              // start()：作为独立任务启动。
	{
		assert(!series_of(this));                             // 断言当前任务还没有被放入任何 SeriesWork。
		Workflow::start_series_work(this, nullptr);           // 创建一个只包含当前任务的串行流并启动。
	}

	void dismiss()                                            // dismiss()：放弃一个尚未启动的线程任务。
	{
		assert(!series_of(this));                             // 只能放弃尚未挂入 SeriesWork 的任务。
		delete this;                                          // 删除任务对象。
	}

public:
	INPUT *get_input() { return &this->input; }                // get_input()：返回输入对象地址，用户可以填充输入参数。
	OUTPUT *get_output() { return &this->output; }             // get_output()：返回输出对象地址，routine 会写入结果。

	const INPUT *get_input() const { return &this->input; }    // const 版本：只读输入。
	const OUTPUT *get_output() const { return &this->output; } // const 版本：只读输出。

public:
	void *user_data;                                           // user_data：用户自定义指针，可挂载任意上下文。

public:
	int get_state() const { return this->state; }              // get_state()：获取任务状态。
	int get_error() const { return this->error; }              // get_error()：获取错误码。

public:
	void set_callback(std::function<void (WFThreadTask<INPUT, OUTPUT> *)> cb) // set_callback(cb)：设置线程任务结束回调。
	{
		this->callback = std::move(cb);                       // 移动保存 callback，减少拷贝成本。
	}

protected:
	virtual SubTask *done()                                    // done()：线程任务完成后的收尾逻辑。
	{
		SeriesWork *series = series_of(this);                  // 找到当前任务所属的串行流。

		if (this->callback)                                    // 如果用户设置了回调。
			this->callback(this);                              // 执行回调，让用户读取 output/state/error。

		delete this;                                           // 当前任务生命周期结束，删除自身。
		return series->pop();                                  // 从串行流中取下一个任务继续执行。
	}

protected:
	INPUT input;                                               // input：线程任务输入数据。
	OUTPUT output;                                             // output：线程任务输出数据。
	std::function<void (WFThreadTask<INPUT, OUTPUT> *)> callback; // callback：线程任务完成回调。

public:
	WFThreadTask(ExecQueue *queue, Executor *executor,          // 构造函数参数 queue：执行队列；executor：线程池执行器。
				 std::function<void (WFThreadTask<INPUT, OUTPUT> *)>&& cb) :
		ExecRequest(queue, executor),                          // 初始化 ExecRequest，让任务知道交给哪个队列和执行器。
		callback(std::move(cb))                                // 保存回调。
	{
		this->user_data = NULL;                                // user_data 初始化为空。
		this->state = WFT_STATE_UNDEFINED;                     // 初始状态为未运行。
		this->error = 0;                                       // 初始错误码为 0。
	}

protected:
	virtual ~WFThreadTask() { }                                // 析构保护：用户通常不直接 delete，任务完成后自删除。
};

template<class REQ, class RESP>                               // REQ：请求消息类型；RESP：响应消息类型。
class WFNetworkTask : public CommRequest                       // WFNetworkTask：HTTP/Redis/MySQL/DNS 网络任务共同基类。
{
public:
	void start()                                               // start()：客户端网络任务独立启动。
	{
		assert(!series_of(this));                              // 确保任务尚未属于任何 SeriesWork。
		Workflow::start_series_work(this, nullptr);            // 创建并启动一个只包含当前任务的串行流。
	}

	void dismiss()                                             // dismiss()：放弃未启动的客户端网络任务。
	{
		assert(!series_of(this));                              // 确保任务还没放入流程。
		delete this;                                           // 删除任务对象。
	}

public:
	REQ *get_req() { return &this->req; }                      // get_req()：获取请求对象；HTTP/Redis/MySQL 命令都往这里写。
	RESP *get_resp() { return &this->resp; }                   // get_resp()：获取响应对象；callback 里从这里读返回结果。

	const REQ *get_req() const { return &this->req; }           // const 版本：只读请求对象。
	const RESP *get_resp() const { return &this->resp; }        // const 版本：只读响应对象。

public:
	void *user_data;                                           // user_data：用户自定义上下文，比如图片 id、请求上下文等。

public:
	int get_state() const { return this->state; }              // get_state()：获取任务状态。
	int get_error() const { return this->error; }              // get_error()：获取错误码。

	int get_timeout_reason() const { return this->timeout_reason; } // get_timeout_reason()：当 error 是 ETIMEDOUT 时，查看超时阶段。

	long long get_task_seq() const                             // get_task_seq()：获取当前连接上的任务序号。
	{
		if (!this->target)                                     // 如果 target 为空，说明任务还没有连接目标。
		{
			errno = ENOTCONN;                                  // 设置 errno 为未连接。
			return -1;                                         // 返回 -1 表示失败。
		}

		return this->get_seq();                                // 返回 CommSession 中维护的序号。
	}

	int get_peer_addr(struct sockaddr *addr, socklen_t *addrlen) const; // get_peer_addr()：获取对端地址；实现放在 WFTask.inl。

	virtual WFConnection *get_connection() const = 0;           // get_connection()：获取底层连接；客户端/服务端任务实现不同。

public:
	void set_send_timeout(int timeout) { this->send_timeo = timeout; }       // 设置完整发送超时，单位毫秒，-1 表示无限。
	void set_receive_timeout(int timeout) { this->receive_timeo = timeout; } // 设置完整接收超时，客户端常用。
	void set_keep_alive(int timeout) { this->keep_alive_timeo = timeout; }   // 设置连接保活时间。
	void set_watch_timeout(int timeout) { this->watch_timeo = timeout; }     // 设置首包等待超时。

public:
	void noreply()                                             // noreply()：服务端任务调用，表示不需要返回响应。
	{
		if (this->state == WFT_STATE_TOREPLY)                  // 只有任务处于待回复状态时才能切到 NOREPLY。
			this->state = WFT_STATE_NOREPLY;                   // 设置状态为不回复。
	}

	virtual int push(const void *buf, size_t size)              // push(buf, size)：服务端同步推送一段响应数据。
	{
		if (this->state != WFT_STATE_TOREPLY &&
			this->state != WFT_STATE_NOREPLY)                  // 如果不是服务端可回复/不回复阶段。
		{
			errno = ENOENT;                                    // 设置错误码，表示当前阶段不能 push。
			return -1;                                         // 返回失败。
		}

		return this->scheduler->push(buf, size, this);         // 交给调度器把数据推到当前连接。
	}

	bool closed() const                                        // closed()：检查连接在回复前是否已经关闭。
	{
		switch (this->state)                                   // 根据当前任务状态判断。
		{
		case WFT_STATE_UNDEFINED:                              // 未运行状态。
			return false;                                      // 未运行不能认为已关闭。
		case WFT_STATE_TOREPLY:                                // 待回复状态。
		case WFT_STATE_NOREPLY:                                // 不回复状态。
			return !this->target->has_idle_conn();             // 如果 target 没有 idle 连接，认为连接关闭。
		default:                                               // 其他状态一般表示任务已结束或异常。
			return true;                                       // 认为连接不可继续使用。
		}
	}

public:
	void set_prepare(std::function<void (WFNetworkTask<REQ, RESP> *)> prep) // set_prepare(prep)：设置发送前准备回调。
	{
		this->prepare = std::move(prep);                       // 保存 prepare；连接建立后、发送前会调用。
	}

public:
	void set_callback(std::function<void (WFNetworkTask<REQ, RESP> *)> cb) // set_callback(cb)：设置任务完成回调。
	{
		this->callback = std::move(cb);                        // 保存 callback；完成后可读取 resp/state/error。
	}

protected:
	virtual int send_timeout() { return this->send_timeo; }       // CommSession 调用：返回发送超时。
	virtual int receive_timeout() { return this->receive_timeo; } // CommSession 调用：返回接收超时。
	virtual int keep_alive_timeout() { return this->keep_alive_timeo; } // CommSession 调用：返回 keep-alive 超时。
	virtual int first_timeout() { return this->watch_timeo; }     // CommSession 调用：返回首包等待超时。

protected:
	int send_timeo;                                            // send_timeo：完整发送超时。
	int receive_timeo;                                         // receive_timeo：完整接收超时。
	int keep_alive_timeo;                                      // keep_alive_timeo：连接保活时间。
	int watch_timeo;                                           // watch_timeo：首包等待超时。
	REQ req;                                                   // req：请求协议对象，例如 HttpRequest、RedisRequest、MySQLRequest。
	RESP resp;                                                 // resp：响应协议对象，例如 HttpResponse、RedisResponse、MySQLResponse。
	std::function<void (WFNetworkTask<REQ, RESP> *)> prepare;  // prepare：发送前准备回调。
	std::function<void (WFNetworkTask<REQ, RESP> *)> callback; // callback：任务结束回调。

protected:
	WFNetworkTask(CommSchedObject *object, CommScheduler *scheduler, // 构造函数：object 是目标调度对象，scheduler 是通信调度器。
				  std::function<void (WFNetworkTask<REQ, RESP> *)>&& cb) :
		CommRequest(object, scheduler),                         // 初始化 CommRequest，让网络任务具备通信调度能力。
		callback(std::move(cb))                                 // 保存完成回调。
	{
		this->send_timeo = -1;                                  // 默认发送完整消息不设总超时。
		this->receive_timeo = -1;                               // 默认接收完整响应不设总超时。
		this->keep_alive_timeo = 0;                             // 默认不额外保持连接，具体协议可调整。
		this->watch_timeo = 0;                                  // 默认不设置首包等待超时。
		this->target = NULL;                                    // 尚未选中通信目标。
		this->timeout_reason = TOR_NOT_TIMEOUT;                 // 默认不是超时。
		this->user_data = NULL;                                 // 用户上下文为空。
		this->state = WFT_STATE_UNDEFINED;                      // 初始状态为未运行。
		this->error = 0;                                        // 初始错误码为 0。
	}

	virtual ~WFNetworkTask() { }                                // 析构保护：任务通常由框架在 done() 中删除。
};

class WFTimerTask : public SleepRequest                        // WFTimerTask：定时器任务，底层基于 SleepRequest。
{
public:
	void start()                                               // start()：独立启动定时器任务。
	{
		assert(!series_of(this));                              // 确保未挂入其他串行流。
		Workflow::start_series_work(this, nullptr);            // 放入单任务串行流并启动。
	}

	void dismiss()                                             // dismiss()：放弃未启动定时器。
	{
		assert(!series_of(this));                              // 确保任务未被编排。
		delete this;                                           // 删除任务对象。
	}

public:
	void *user_data;                                           // user_data：用户上下文。

public:
	int get_state() const { return this->state; }              // get_state()：获取定时器任务状态。
	int get_error() const { return this->error; }              // get_error()：获取错误码。

public:
	void set_callback(std::function<void (WFTimerTask *)> cb)  // set_callback(cb)：设置定时结束回调。
	{
		this->callback = std::move(cb);                        // 保存回调。
	}

protected:
	virtual SubTask *done()                                    // done()：定时器触发后的收尾逻辑。
	{
		SeriesWork *series = series_of(this);                  // 找到所属串行流。

		if (this->callback)                                    // 如果设置了回调。
			this->callback(this);                              // 执行定时器回调。

		delete this;                                           // 删除当前任务。
		return series->pop();                                  // 返回串行流里的下一个任务。
	}

protected:
	std::function<void (WFTimerTask *)> callback;              // callback：定时器完成回调。

public:
	WFTimerTask(CommScheduler *scheduler,                       // scheduler：定时器也通过通信调度器注册 sleep。
				std::function<void (WFTimerTask *)> cb) :
		SleepRequest(scheduler),                               // 初始化 SleepRequest。
		callback(std::move(cb))                                // 保存回调。
	{
		this->user_data = NULL;                                // 用户上下文为空。
		this->state = WFT_STATE_UNDEFINED;                     // 初始状态未定义。
		this->error = 0;                                       // 初始错误码 0。
	}

protected:
	virtual ~WFTimerTask() { }                                 // 析构保护。
};

template<class ARGS>                                          // ARGS：文件 IO 参数类型，如 FileIOArgs/FileVIOArgs/FileSyncArgs。
class WFFileTask : public IORequest                           // WFFileTask：文件异步 IO 任务。
{
public:
	void start()                                               // start()：独立启动文件任务。
	{
		assert(!series_of(this));                              // 确保未加入其他流程。
		Workflow::start_series_work(this, nullptr);            // 创建单任务串行流并启动。
	}

	void dismiss()                                             // dismiss()：放弃未启动文件任务。
	{
		assert(!series_of(this));                              // 确认未挂入流程。
		delete this;                                           // 删除任务对象。
	}

public:
	ARGS *get_args() { return &this->args; }                   // get_args()：获取文件任务参数，如 fd、buf、count、offset。

	const ARGS *get_args() const { return &this->args; }        // const 版本：只读参数。

	long get_retval() const                                    // get_retval()：获取文件 IO 系统调用返回值。
	{
		if (this->state == WFT_STATE_SUCCESS)                  // 只有任务成功时返回真实结果。
			return this->get_res();                            // get_res() 来自 IORequest，表示 read/write 返回字节数等。
		else                                                   // 如果任务失败。
			return -1;                                         // 返回 -1，与系统调用失败习惯一致。
	}

public:
	void *user_data;                                           // user_data：用户上下文，例如图片 id 或请求对象。

public:
	int get_state() const { return this->state; }              // get_state()：获取文件任务状态。
	int get_error() const { return this->error; }              // get_error()：获取错误码。

public:
	void set_callback(std::function<void (WFFileTask<ARGS> *)> cb) // set_callback(cb)：设置文件任务结束回调。
	{
		this->callback = std::move(cb);                        // 保存回调。
	}

protected:
	virtual SubTask *done()                                    // done()：文件 IO 完成后的收尾逻辑。
	{
		SeriesWork *series = series_of(this);                  // 找到所属串行流。

		if (this->callback)                                    // 如果有回调。
			this->callback(this);                              // 执行回调，用户可读取 get_retval()/state/error。

		delete this;                                           // 删除当前文件任务。
		return series->pop();                                  // 调度串行流下一个任务。
	}

protected:
	ARGS args;                                                 // args：文件 IO 参数。
	std::function<void (WFFileTask<ARGS> *)> callback;         // callback：文件 IO 完成回调。

public:
	WFFileTask(IOService *service,                              // service：文件 IO 服务对象。
			   std::function<void (WFFileTask<ARGS> *)>&& cb) :
		IORequest(service),                                    // 初始化 IORequest，让任务能提交给 IOService。
		callback(std::move(cb))                                // 保存回调。
	{
		this->user_data = NULL;                                // 用户上下文为空。
		this->state = WFT_STATE_UNDEFINED;                     // 初始状态未运行。
		this->error = 0;                                       // 初始错误码 0。
	}

protected:
	virtual ~WFFileTask() { }                                  // 析构保护。
};

/*
  下面这些 Generic 类更偏任务控制工具。
  随机图库项目第一阶段不一定用到，但它们能体现 Workflow 把“等待、计数、条件、重复执行”等控制逻辑也统一成任务。
*/

class WFGenericTask : public SubTask                           // WFGenericTask：最简单的通用任务，dispatch 后立即完成。
{
public:
	void start()                                               // start()：独立启动通用任务。
	{
		assert(!series_of(this));                              // 确保未挂入流程。
		Workflow::start_series_work(this, nullptr);            // 放入串行流启动。
	}

	void dismiss()                                             // dismiss()：放弃未启动任务。
	{
		assert(!series_of(this));                              // 确认未编排。
		delete this;                                           // 删除对象。
	}

public:
	void *user_data;                                           // user_data：用户上下文。

public:
	int get_state() const { return this->state; }              // 获取任务状态。
	int get_error() const { return this->error; }              // 获取错误码。

protected:
	virtual void dispatch()                                    // dispatch()：启动任务。
	{
		this->subtask_done();                                  // GenericTask 没有异步操作，启动后立即完成。
	}

	virtual SubTask *done()                                    // done()：完成后的收尾。
	{
		SeriesWork *series = series_of(this);                  // 找到所属串行流。
		delete this;                                           // 删除自己。
		return series->pop();                                  // 调度下一个任务。
	}

protected:
	int state;                                                 // state：任务状态。
	int error;                                                 // error：错误码。

public:
	WFGenericTask()                                            // 构造函数。
	{
		this->user_data = NULL;                                // 用户上下文为空。
		this->state = WFT_STATE_UNDEFINED;                     // 初始状态未定义。
		this->error = 0;                                       // 初始错误码 0。
	}

protected:
	virtual ~WFGenericTask() { }                               // 析构保护。
};

class WFCounterTask : public WFGenericTask                     // WFCounterTask：计数器任务，count() 被调用到目标次数才完成。
{
public:
	virtual void count()                                       // count()：计数一次。
	{
		if (--this->value == 0)                                // 原子计数减 1；减到 0 表示条件满足。
		{
			this->state = WFT_STATE_SUCCESS;                   // 设置任务成功。
			this->subtask_done();                              // 推进工作流。
		}
	}

public:
	void set_callback(std::function<void (WFCounterTask *)> cb) // 设置计数完成回调。
	{
		this->callback = std::move(cb);                        // 保存回调。
	}

protected:
	virtual void dispatch()                                    // dispatch()：启动计数器。
	{
		this->WFCounterTask::count();                          // 启动时先 count 一次，与构造时 target_value+1 配套。
	}

	virtual SubTask *done()                                    // done()：计数完成后的收尾。
	{
		SeriesWork *series = series_of(this);                  // 找到所属串行流。

		if (this->callback)                                    // 如果有回调。
			this->callback(this);                              // 执行回调。

		delete this;                                           // 删除任务。
		return series->pop();                                  // 返回下一个任务。
	}

protected:
	std::atomic<unsigned int> value;                           // value：剩余计数，原子类型保证多线程安全。
	std::function<void (WFCounterTask *)> callback;            // callback：计数完成回调。

public:
	WFCounterTask(unsigned int target_value,                    // target_value：目标计数次数。
				  std::function<void (WFCounterTask *)>&& cb) :
		value(target_value + 1),                               // 加 1 是因为 dispatch() 自己会先调用一次 count()。
		callback(std::move(cb))                                // 保存回调。
	{
	}

protected:
	virtual ~WFCounterTask() { }                               // 析构保护。
};

/*
  后续类保持原始结构并补充关键注释。
  如果你的项目第一阶段只做 HTTP + Redis + MySQL + FileIO，可以先不用深挖这些控制任务。
*/

class WFMailboxTask : public WFGenericTask
{
public:
	virtual void send(void *msg)                               // send(msg)：向 mailbox 发送一个消息。
	{
		*this->mailbox = msg;                                  // 把消息指针写入 mailbox 指向的位置。
		if (this->flag.exchange(true))                         // flag 第一次置 true 只表示一方到达；第二方到达才完成。
		{
			this->state = WFT_STATE_SUCCESS;                   // 标记成功。
			this->subtask_done();                              // 推进工作流。
		}
	}

	void **get_mailbox() const { return this->mailbox; }        // 获取 mailbox 地址。

public:
	void set_callback(std::function<void (WFMailboxTask *)> cb) { this->callback = std::move(cb); } // 设置回调。

protected:
	virtual void dispatch()
	{
		if (this->flag.exchange(true))                         // 如果 send() 已经先到达，则 dispatch() 到达后任务完成。
		{
			this->state = WFT_STATE_SUCCESS;
			this->subtask_done();
		}
	}

	virtual SubTask *done()
	{
		SeriesWork *series = series_of(this);
		if (this->callback)
			this->callback(this);
		delete this;
		return series->pop();
	}

protected:
	void **mailbox;                                            // mailbox：保存消息指针的地址。
	std::atomic<bool> flag;                                    // flag：同步 send 和 dispatch 两个事件。
	std::function<void (WFMailboxTask *)> callback;            // callback：完成回调。

public:
	WFMailboxTask(void **mailbox, std::function<void (WFMailboxTask *)>&& cb) :
		flag(false), callback(std::move(cb))
	{
		this->mailbox = mailbox;                               // 使用用户提供的 mailbox 存储位置。
	}

	WFMailboxTask(std::function<void (WFMailboxTask *)>&& cb) :
		flag(false), callback(std::move(cb))
	{
		this->mailbox = &this->user_data;                      // 默认把 user_data 当 mailbox。
	}

protected:
	virtual ~WFMailboxTask() { }
};

class WFSelectorTask : public WFGenericTask
{
public:
	virtual int submit(void *msg)                              // submit(msg)：候选方提交一个结果，谁先提交成功谁被选中。
	{
		void *tmp = NULL;                                      // tmp：compare_exchange 期望 message 仍为 NULL。
		int ret = 0;                                           // ret：是否成功选中当前 msg。

		if (this->message.compare_exchange_strong(tmp, msg) && msg) // 如果 message 原来为空且 msg 非空，则当前 msg 成为结果。
		{
			ret = 1;                                           // 标记当前提交被选中。
			if (this->flag.exchange(true))                     // 如果 selector 已经 dispatch，则可以完成任务。
			{
				this->state = WFT_STATE_SUCCESS;
				this->subtask_done();
			}
		}

		if (--this->nleft == 0)                                // 每个候选方结束时减 1；如果所有候选方都结束。
		{
			if (!this->message)                                // 如果没有任何候选方提交有效消息。
			{
				this->state = WFT_STATE_SYS_ERROR;             // 标记系统错误。
				this->error = ENOMSG;                          // ENOMSG 表示没有消息。
				this->subtask_done();                          // 推进流程。
			}

			delete this;                                       // selector 自身生命周期结束。
		}

		return ret;                                            // 返回是否选中。
	}

	void *get_message() const { return this->message; }         // 获取被选中的消息。

public:
	void set_callback(std::function<void (WFSelectorTask *)> cb) { this->callback = std::move(cb); } // 设置回调。

protected:
	virtual void dispatch()
	{
		if (this->flag.exchange(true))                         // 如果已有候选消息，则 dispatch 时完成。
		{
			this->state = WFT_STATE_SUCCESS;
			this->subtask_done();
		}

		if (--this->nleft == 0)                                // dispatch 自身也占一个 nleft 计数。
		{
			if (!this->message)
			{
				this->state = WFT_STATE_SYS_ERROR;
				this->error = ENOMSG;
				this->subtask_done();
			}

			delete this;
		}
	}

	virtual SubTask *done()
	{
		SeriesWork *series = series_of(this);
		if (this->callback)
			this->callback(this);
		return series->pop();
	}

protected:
	std::atomic<void *> message;                               // message：第一个成功提交的消息。
	std::atomic<bool> flag;                                    // flag：同步 dispatch 和 submit。
	std::atomic<size_t> nleft;                                 // nleft：剩余候选方数量加上 dispatch 自身。
	std::function<void (WFSelectorTask *)> callback;           // callback：完成回调。

public:
	WFSelectorTask(size_t candidates, std::function<void (WFSelectorTask *)>&& cb) :
		message(NULL), flag(false), nleft(candidates + 1), callback(std::move(cb))
	{
	}

protected:
	virtual ~WFSelectorTask() { }
};

class WFConditional : public WFGenericTask
{
public:
	virtual void signal(void *msg)                             // signal(msg)：外部条件触发，传入消息。
	{
		*this->msgbuf = msg;                                   // 保存消息。
		if (this->flag.exchange(true))                         // 如果 dispatch 已经到达，则条件满足。
			this->subtask_done();
	}

protected:
	virtual void dispatch()
	{
		series_of(this)->push_front(this->task);               // 把被保护的 task 插到当前任务后面。
		this->task = NULL;                                     // task 已交给 SeriesWork，避免析构重复删除。
		if (this->flag.exchange(true))                         // 如果 signal 已经先发生，则任务完成。
			this->subtask_done();
	}

protected:
	std::atomic<bool> flag;                                    // flag：同步 dispatch 和 signal。
	SubTask *task;                                             // task：条件满足后要执行的任务。
	void **msgbuf;                                             // msgbuf：保存 signal 传入消息的位置。

public:
	WFConditional(SubTask *task, void **msgbuf) : flag(false)
	{
		this->task = task;
		this->msgbuf = msgbuf;
	}

	WFConditional(SubTask *task) : flag(false)
	{
		this->task = task;
		this->msgbuf = &this->user_data;
	}

protected:
	virtual ~WFConditional()
	{
		delete this->task;                                     // 如果 task 还没交给 SeriesWork，则析构时删除。
	}
};

class WFGoTask : public ExecRequest                            // WFGoTask：更轻量的 go 风格计算任务。
{
public:
	void start() { assert(!series_of(this)); Workflow::start_series_work(this, nullptr); } // 独立启动。
	void dismiss() { assert(!series_of(this)); delete this; }   // 放弃未启动任务。

public:
	void *user_data;                                           // 用户上下文。

public:
	int get_state() const { return this->state; }              // 获取状态。
	int get_error() const { return this->error; }              // 获取错误码。

public:
	void set_callback(std::function<void (WFGoTask *)> cb) { this->callback = std::move(cb); } // 设置回调。

protected:
	virtual SubTask *done()
	{
		SeriesWork *series = series_of(this);
		if (this->callback)
			this->callback(this);
		delete this;
		return series->pop();
	}

protected:
	std::function<void (WFGoTask *)> callback;                 // 完成回调。

public:
	WFGoTask(ExecQueue *queue, Executor *executor) : ExecRequest(queue, executor)
	{
		this->user_data = NULL;
		this->state = WFT_STATE_UNDEFINED;
		this->error = 0;
	}

protected:
	virtual ~WFGoTask() { }
};

class WFRepeaterTask : public WFGenericTask                    // WFRepeaterTask：重复生成任务，直到 create 返回 NULL。
{
public:
	void set_create(std::function<SubTask *(WFRepeaterTask *)> create) { this->create = std::move(create); } // 设置生成函数。

public:
	void set_callback(std::function<void (WFRepeaterTask *)> cb) { this->callback = std::move(cb); } // 设置结束回调。

protected:
	virtual void dispatch()
	{
		SubTask *task = this->create(this);                    // 调用 create 生成下一轮任务。

		if (task)                                              // 如果生成了任务。
		{
			series_of(this)->push_front(this);                 // 把 repeater 自己放回队头，等待下一轮。
			series_of(this)->push_front(task);                 // 把新任务放到 repeater 前面，先执行新任务。
		}
		else                                                   // 如果没有新任务。
			this->state = WFT_STATE_SUCCESS;                   // 标记重复过程结束。

		this->subtask_done();                                  // 推进当前流程。
	}

	virtual SubTask *done()
	{
		SeriesWork *series = series_of(this);
		if (this->state != WFT_STATE_UNDEFINED)                // state 被设置说明重复已经结束。
		{
			if (this->callback)
				this->callback(this);
			delete this;
		}
		return series->pop();
	}

protected:
	std::function<SubTask *(WFRepeaterTask *)> create;         // create：每轮生成新任务。
	std::function<void (WFRepeaterTask *)> callback;           // callback：重复结束回调。

public:
	WFRepeaterTask(std::function<SubTask *(WFRepeaterTask *)>&& create,
				   std::function<void (WFRepeaterTask *)>&& cb) :
		create(std::move(create)), callback(std::move(cb))
	{
	}

protected:
	virtual ~WFRepeaterTask() { }
};

class WFModuleTask : public ParallelTask, protected SeriesWork // WFModuleTask：把一段子 SeriesWork 封装成一个模块任务。
{
public:
	void start() { assert(!series_of(this)); Workflow::start_series_work(this, nullptr); } // 独立启动模块。
	void dismiss() { assert(!series_of(this)); delete this; }   // 放弃未启动模块。

public:
	SeriesWork *sub_series() { return this; }                   // 获取模块内部的子串行流。
	const SeriesWork *sub_series() const { return this; }       // const 版本。

public:
	void *user_data;                                           // 用户上下文。

public:
	void set_callback(std::function<void (const WFModuleTask *)> cb) { this->callback = std::move(cb); } // 设置模块完成回调。

protected:
	virtual SubTask *done()
	{
		SeriesWork *series = series_of(this);                  // 外层串行流。
		if (this->callback)
			this->callback(this);
		delete this;
		return series->pop();
	}

protected:
	SubTask *first;                                            // 模块内部第一个任务。
	std::function<void (const WFModuleTask *)> callback;       // 模块完成回调。

public:
	WFModuleTask(SubTask *first, std::function<void (const WFModuleTask *)>&& cb) :
		ParallelTask(&this->first, 1),                         // 把模块内部 series 当成一个并行任务分支。
		SeriesWork(first, nullptr),                            // 初始化内部 SeriesWork。
		callback(std::move(cb))
	{
		this->first = first;                                   // 保存内部第一个任务。
		this->set_in_parallel(this);                           // 标记内部 SeriesWork 属于当前模块。
		this->user_data = NULL;                                // 初始化用户上下文。
	}

protected:
	virtual ~WFModuleTask()
	{
		if (!this->is_finished())                              // 如果模块内部流程没完成。
			this->dismiss_recursive();                         // 递归清理内部任务。
	}
};

#include "WFTask.inl"                                         // 引入模板实现文件；模板类实现必须对使用方可见。

#endif                                                        // 结束头文件保护宏。
