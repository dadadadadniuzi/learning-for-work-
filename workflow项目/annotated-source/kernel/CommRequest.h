/*
  注释版源码文件：CommRequest.h

  原始文件位置：
  workflow-master/src/kernel/CommRequest.h

  本文件用途：
  CommRequest 把“一个网络通信会话”包装成 Workflow 可调度的 SubTask。
  HTTP、Redis、MySQL 等网络任务的底层，都会走类似这条路径。

  关键继承关系：
  CommRequest : public SubTask, public CommSession

  含义：
  1. 继承 SubTask：说明它是一个可以被 Workflow 编排的任务。
  2. 继承 CommSession：说明它同时也是一次通信会话，可以交给 Communicator 做网络收发。
*/

#ifndef _COMMREQUEST_H_                                      // 头文件保护宏：防止重复 include。
#define _COMMREQUEST_H_                                      // 定义头文件保护宏。

#include <errno.h>                                           // 引入 errno；网络调度失败时会读取 errno 保存错误原因。
#include <stddef.h>                                          // 引入 NULL、size_t 等基础类型定义。
#include "SubTask.h"                                         // 引入 SubTask；CommRequest 要继承 SubTask。
#include "Communicator.h"                                    // 引入 CommSession、CommTarget 等通信会话相关类型。
#include "CommScheduler.h"                                   // 引入 CommScheduler、CommSchedObject 等调度器相关类型。

class CommRequest : public SubTask, public CommSession        // CommRequest 同时是任务和通信会话。
{
public:                                                      // public 区域：构造函数和外部可调用接口。
	CommRequest(CommSchedObject *object, CommScheduler *scheduler) // 构造函数：创建一个通信请求任务。
	{                                                        // 输入参数 object：调度对象，表示请求应该从哪个目标或目标组获取连接。
		                                                     // 输入参数 scheduler：通信调度器，负责选择连接并把会话交给 Communicator。
		this->scheduler = scheduler;                         // 保存 scheduler；dispatch() 时会调用 scheduler->request() 发起请求。
		this->object = object;                               // 保存 object；调度器会从 object 中 acquire 一个 CommTarget。
		this->wait_timeout = 0;                              // 默认等待连接超时为 0；表示如果没有可用连接，默认不等待。
	}

	CommSchedObject *get_request_object() const              // get_request_object()：获取当前请求使用的调度对象。
	{
		return this->object;                                 // 返回 object；外部可以观察当前任务绑定到哪个目标选择对象。
	}

	void set_request_object(CommSchedObject *object)          // set_request_object(object)：修改当前请求使用的调度对象。
	{                                                        // 输入参数 object：新的调度对象。
		this->object = object;                               // 保存新的 object；后续 dispatch() 会使用这个新对象获取目标连接。
	}

	int get_wait_timeout() const                             // get_wait_timeout()：获取同步等待连接的超时时间。
	{
		return this->wait_timeout;                           // 返回 wait_timeout，单位通常是毫秒。
	}

	void set_wait_timeout(int timeout)                       // set_wait_timeout(timeout)：设置等待可用连接的超时时间。
	{                                                        // 输入参数 timeout：等待时间，单位毫秒；-1 通常表示无限等待。
		this->wait_timeout = timeout;                        // 保存 timeout；dispatch() 调度请求时会传给 scheduler->request()。
	}

public:                                                      // public 区域：实现 SubTask 的启动函数。
	virtual void dispatch()                                  // dispatch()：启动通信任务。
	{                                                        // 没有输入参数；任务需要的 scheduler/object/wait_timeout 都是成员变量。
		if (this->scheduler->request(this, this->object, this->wait_timeout,
									 &this->target) < 0)     // 调用调度器发起请求。
		{                                                    // 参数 this：当前 CommRequest，同时也是 CommSession。
			                                                 // 参数 object：用于选择目标连接。
			                                                 // 参数 wait_timeout：等待连接的超时时间。
			                                                 // 参数 &target：输出参数，调度器成功后会把选中的目标写入 this->target。
			this->handle(CS_STATE_ERROR, errno);             // 如果 request() 失败，调用 handle() 记录错误并结束任务。
			                                                 // CS_STATE_ERROR 表示通信层错误；errno 是系统错误码。
		}
	}

protected:                                                   // protected 区域：子类可以读取任务状态。
	int state;                                               // state：任务结束状态，比如成功、系统错误、被停止等。
	int error;                                               // error：任务错误码；当 state 表示错误时，用它保存具体原因。

protected:                                                   // protected 区域：通信目标和超时原因。
	CommTarget *target;                                      // target：实际被选中的通信目标；成功 request 后由调度器写入。
#define TOR_NOT_TIMEOUT			0                           // TOR_NOT_TIMEOUT：不是超时错误。
#define TOR_WAIT_TIMEOUT		1                           // TOR_WAIT_TIMEOUT：等待连接可用时超时。
#define TOR_CONNECT_TIMEOUT		2                           // TOR_CONNECT_TIMEOUT：建立连接阶段超时。
#define TOR_TRANSMIT_TIMEOUT	3                           // TOR_TRANSMIT_TIMEOUT：发送或接收阶段超时。
	int timeout_reason;                                      // timeout_reason：当 error 是 ETIMEDOUT 时，用它进一步区分超时发生在哪个阶段。

protected:                                                   // protected 区域：请求调度所需成员。
	int wait_timeout;                                        // wait_timeout：等待可用连接的超时时间；传入 scheduler->request()。
	CommSchedObject *object;                                 // object：调度对象，可能是单目标，也可能是目标组。
	CommScheduler *scheduler;                                // scheduler：通信调度器，负责目标获取和网络请求提交。

protected:                                                   // protected 区域：通信结果处理。
	virtual void handle(int state, int error);                // handle(state, error)：通信完成或失败时调用。
	                                                         // 输入参数 state：通信层状态。
	                                                         // 输入参数 error：具体错误码。
	                                                         // 主要目的：保存状态、判断超时原因、调用 subtask_done() 推进工作流。
};

#endif                                                       // 结束头文件保护宏。
