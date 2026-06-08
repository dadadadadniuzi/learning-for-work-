/*
  注释版源码文件：ExecRequest.h

  原始文件位置：
  workflow-master/src/kernel/ExecRequest.h

  本文件用途：
  ExecRequest 把 Executor 的计算会话包装成 Workflow 的 SubTask。

  对随机图库项目的意义：
  如果你后面要做图片压缩、缩略图生成、图片哈希计算，这些 CPU 密集型逻辑不应该放在网络回调里阻塞，
  可以用 WFGoTask / WFThreadTask 交给 Executor 线程池执行。
*/

#ifndef _EXECREQUEST_H_                                       // 头文件保护宏。
#define _EXECREQUEST_H_                                       // 定义头文件保护宏。

#include <errno.h>                                            // errno。
#include "SubTask.h"                                          // SubTask。
#include "Executor.h"                                         // ExecSession/Executor/ExecQueue。

class ExecRequest : public SubTask, public ExecSession         // ExecRequest 同时是 Workflow 任务和线程池执行会话。
{
public:
	ExecRequest(ExecQueue *queue, Executor *executor)          // 构造函数。
	{
		this->executor = executor;                             // 保存执行器，dispatch 时会提交给它。
		this->queue = queue;                                   // 保存执行队列。
	}

	ExecQueue *get_request_queue() const { return this->queue; } // 获取当前执行队列。
	void set_request_queue(ExecQueue *queue) { this->queue = queue; } // 修改执行队列。

public:
	virtual void dispatch()                                    // dispatch()：启动计算任务。
	{
		if (this->executor->request(this, this->queue) < 0)     // 把当前对象作为 ExecSession 提交给 Executor。
			this->handle(ES_STATE_ERROR, errno);                // 提交失败则直接按错误完成。
	}

protected:
	int state;                                                 // state：任务执行状态。
	int error;                                                 // error：错误码。

protected:
	ExecQueue *queue;                                          // queue：目标执行队列。
	Executor *executor;                                        // executor：目标执行器。

protected:
	virtual void handle(int state, int error)                  // handle(state,error)：计算任务完成后的统一处理。
	{
		this->state = state;                                   // 保存状态。
		this->error = error;                                   // 保存错误码。
		this->subtask_done();                                  // 通知 Workflow 任务完成，继续后续任务。
	}
};

#endif                                                        // 结束头文件保护宏。
