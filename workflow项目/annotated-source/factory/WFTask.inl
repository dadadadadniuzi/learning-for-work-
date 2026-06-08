/*
  注释版源码文件：WFTask.inl

  原始文件位置：
  workflow-master/src/factory/WFTask.inl

  本文件用途：
  实现 WFNetworkTask 的模板函数，以及定义客户端/服务端网络任务的具体行为。

  最关键的理解：
  1. WFClientTask 用于 HTTP/Redis/MySQL 客户端请求。
  2. WFServerTask 用于 HTTP Server 收到请求后的服务端任务。
  3. 客户端任务 message_out() 返回请求对象，message_in() 返回响应对象。
  4. 服务端任务 message_in() 返回请求对象，message_out() 返回响应对象。
*/

template<class REQ, class RESP>
int WFNetworkTask<REQ, RESP>::get_peer_addr(struct sockaddr *addr,
											socklen_t *addrlen) const
{
	const struct sockaddr *p;                                  // p：临时保存目标地址指针。
	socklen_t len;                                             // len：目标地址长度。

	if (this->target)                                          // 如果当前任务已经有通信目标。
	{
		this->target->get_addr(&p, &len);                      // 从 target 中取出对端地址和长度。
		if (*addrlen >= len)                                   // 如果用户提供的 addr 缓冲区足够大。
		{
			memcpy(addr, p, len);                              // 把目标地址复制到用户传入的 addr。
			*addrlen = len;                                    // 把真实地址长度写回 addrlen。
			return 0;                                          // 返回 0 表示成功。
		}

		errno = ENOBUFS;                                       // 缓冲区太小，设置 errno。
	}
	else                                                       // 如果 target 为空。
		errno = ENOTCONN;                                      // 设置 errno 为未连接。

	return -1;                                                 // 返回 -1 表示失败。
}

template<class REQ, class RESP>
class WFClientTask : public WFNetworkTask<REQ, RESP>           // WFClientTask：客户端网络任务，比如 HTTP 客户端、Redis 客户端、MySQL 客户端。
{
protected:
	virtual CommMessageOut *message_out()                      // message_out()：告诉 Communicator 要发送什么消息。
	{
		if (this->prepare)                                     // 如果用户设置了 prepare 回调。
			this->prepare(this);                               // 在连接建立后、发送前调用，允许用户动态修改请求。

		return &this->req;                                     // 返回请求对象地址；req 必须实现 CommMessageOut 编码接口。
	}

	virtual CommMessageIn *message_in() { return &this->resp; } // message_in()：告诉 Communicator 收到的数据解析到 resp。

protected:
	virtual WFConnection *get_connection() const                // get_connection()：获取当前任务使用的连接。
	{
		CommConnection *conn;                                  // conn：底层通信连接指针。

		if (this->target)                                      // 如果任务已经选中目标。
		{
			conn = this->CommSession::get_connection();        // 从 CommSession 中取底层连接。
			if (conn)                                          // 如果连接存在。
				return (WFConnection *)conn;                   // 转成 Workflow 的 WFConnection 返回。
		}

		errno = ENOTCONN;                                      // 如果没有连接，设置 errno。
		return NULL;                                           // 返回 NULL 表示失败。
	}

protected:
	virtual SubTask *done()                                    // done()：客户端网络任务完成后的收尾。
	{
		SeriesWork *series = series_of(this);                  // 找到当前任务所属串行流。

		if (this->state == WFT_STATE_SYS_ERROR && this->error < 0) // 如果是系统错误且 error 为负数。
		{
			this->state = WFT_STATE_SSL_ERROR;                 // 负数错误在这里被转成 SSL 错误。
			this->error = -this->error;                        // 转为正数 SSL 错误码。
		}

		if (this->callback)                                    // 如果用户设置了完成回调。
			this->callback(this);                              // 调用回调，用户读取 resp/state/error。

		delete this;                                           // 客户端任务完成后删除自己。
		return series->pop();                                  // 返回串行流下一个任务。
	}

public:
	WFClientTask(CommSchedObject *object, CommScheduler *scheduler,
				 std::function<void (WFNetworkTask<REQ, RESP> *)>&& cb) :
		WFNetworkTask<REQ, RESP>(object, scheduler, std::move(cb)) // 调用基类构造，保存调度对象、调度器和回调。
	{
	}

protected:
	virtual ~WFClientTask() { }                                // 析构保护。
};

template<class REQ, class RESP>
class WFServerTask : public WFNetworkTask<REQ, RESP>           // WFServerTask：服务端网络任务，比如 HTTP Server 收到的请求。
{
protected:
	virtual CommMessageOut *message_out()                      // 服务端 message_out()：返回要发给客户端的响应对象。
	{
		if (this->prepare)                                     // 如果设置了发送前 prepare。
			this->prepare(this);                               // 回复前允许用户修改响应。

		return &this->resp;                                    // 服务端发送的是响应 resp。
	}

	virtual CommMessageIn *message_in() { return &this->req; }  // 服务端接收的是请求 req。
	virtual void handle(int state, int error);                  // handle()：服务端通信事件处理，下面有实现。

protected:
	virtual WFConnection *get_connection() const                // get_connection()：服务端在 process 阶段获取连接。
	{
		if (this->processor.task == this)                      // 只有 process 正在执行时允许获取连接。
			return (WFConnection *)this->CommSession::get_connection(); // 返回底层连接。

		errno = this->processor.task ? ENOTCONN : EPERM;       // 如果任务已失效或阶段不对，设置不同 errno。
		return NULL;                                           // 返回 NULL。
	}

protected:
	virtual void dispatch()                                    // dispatch()：服务端任务的“回复阶段”启动函数。
	{
		if (this->state == WFT_STATE_TOREPLY)                  // 如果当前状态是待回复。
		{
			this->processor.task = this;                       // 恢复 processor.task，允许 reply 阶段访问任务。
			if (this->scheduler->reply(this) >= 0)             // 调用调度器发送响应。
				return;                                        // 成功提交回复后直接返回，等待底层完成事件。

			this->state = WFT_STATE_SYS_ERROR;                 // 如果 reply 提交失败，设置系统错误状态。
			this->error = errno;                               // 保存 errno。
		}
		else                                                   // 如果不是待回复状态。
			this->scheduler->shutdown(this);                   // 关闭当前会话。

		this->processor.task = (WFServerTask *)-1;             // 设置特殊值，表示处理器任务失效。
		this->subtask_done();                                  // 推进工作流。
	}

	virtual SubTask *done()                                    // done()：服务端回复完成后的收尾。
	{
		SeriesWork *series = series_of(this);                  // 找到所属串行流。

		if (this->state == WFT_STATE_SYS_ERROR && this->error < 0) // 处理 SSL 负数错误。
		{
			this->state = WFT_STATE_SSL_ERROR;
			this->error = -this->error;
		}

		if (this->callback)                                    // 如果有 callback。
			this->callback(this);                              // 执行服务端任务完成回调。

		this->processor.task = NULL;                           // 清空 processor.task，防止后续访问。
		return series->pop();                                  // 调度串行流下一个任务。
	}

protected:
	class Processor : public SubTask                           // Processor：把用户的 process(task) 包装成一个 SubTask。
	{
	public:
		Processor(WFServerTask *task,
				  std::function<void (WFNetworkTask<REQ, RESP> *)>& proc) :
			process(proc)                                      // 保存用户处理函数引用。
		{
			this->task = task;                                 // 保存当前服务端任务。
		}

		virtual void dispatch()                                // dispatch()：执行用户业务逻辑。
		{
			this->process(this->task);                         // 调用用户传入的 process(task)，比如 HTTP handler。
			this->task = NULL;                                 // 清空 task，作为禁止继续获取连接的标记。
			this->subtask_done();                              // 用户逻辑执行完，推进到下一个任务。
		}

		virtual SubTask *done()                                // done()：Processor 完成后的收尾。
		{
			return series_of(this)->pop();                     // 从当前 SeriesWork 取下一个任务。
		}

		std::function<void (WFNetworkTask<REQ, RESP> *)>& process; // process：用户的服务端处理函数引用。
		WFServerTask *task;                                   // task：当前服务端请求任务。
	} processor;                                              // processor：WFServerTask 内嵌的用户处理任务。

	class Series : public SeriesWork                           // Series：服务端请求内部专用串行流。
	{
	public:
		Series(WFServerTask *task) :
			SeriesWork(&task->processor, nullptr)              // 串行流第一个任务是 processor，也就是用户 process。
		{
			this->set_last_task(task);                         // 最后一个任务设置为 server task 自己，用于回复响应。
			this->task = task;                                 // 保存 task，析构时用于清理。
		}

		virtual ~Series()
		{
			delete this->task;                                 // 删除服务端任务对象。
		}

		WFServerTask *task;                                   // task：服务端请求任务。
	};

public:
	WFServerTask(CommService *service, CommScheduler *scheduler,
				 std::function<void (WFNetworkTask<REQ, RESP> *)>& proc) :
		WFNetworkTask<REQ, RESP>(NULL, scheduler, nullptr),    // 服务端任务不通过 CommSchedObject 发起请求，所以 object 为 NULL。
		processor(this, proc)                                  // 初始化 processor，绑定用户 process。
	{
	}

protected:
	virtual ~WFServerTask()
	{
		if (this->target)                                      // 如果任务已经绑定通信目标。
			((Series *)series_of(&this->processor))->task = NULL; // 避免 Series 析构重复删除 task。
	}
};

template<class REQ, class RESP>
void WFServerTask<REQ, RESP>::handle(int state, int error)     // handle(state, error)：服务端通信事件处理。
{
	if (state == WFT_STATE_TOREPLY)                            // 第一种情况：请求已经完整接收，进入待回复阶段。
	{
		this->state = WFT_STATE_TOREPLY;                       // 保存状态为待回复。
		this->target = this->get_target();                     // 保存当前通信目标。
		new Series(this);                                      // 创建服务端内部串行流：processor -> reply task。
		this->processor.dispatch();                            // 启动 processor，也就是执行用户 process(task)。
	}
	else if (this->state == WFT_STATE_TOREPLY)                 // 第二种情况：之前处于回复阶段，现在收到回复完成或失败事件。
	{
		this->state = state;                                   // 保存最终状态。
		this->error = error;                                   // 保存最终错误码。
		if (error == ETIMEDOUT)                                // 如果是超时。
			this->timeout_reason = TOR_TRANSMIT_TIMEOUT;       // 服务端回复阶段超时属于传输超时。

		this->subtask_done();                                  // 回复阶段完成，推进流程。
	}
	else                                                       // 第三种情况：还没进入业务处理就失败，或者连接异常。
		delete this;                                           // 直接删除任务。
}

