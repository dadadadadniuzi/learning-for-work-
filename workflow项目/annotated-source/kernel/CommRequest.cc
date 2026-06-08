/*
  注释版源码文件：CommRequest.cc

  原始文件位置：
  workflow-master/src/kernel/CommRequest.cc

  本文件用途：
  实现 CommRequest::handle()。
  这个函数是网络任务完成后的统一处理入口之一。

  核心作用：
  1. 保存任务状态 state。
  2. 保存错误码 error。
  3. 如果是超时错误，判断超时发生在等待连接、建立连接还是传输阶段。
  4. 调用 subtask_done()，把控制权交还给 Workflow 任务编排系统。
*/

#include <errno.h>                         // 引入 ETIMEDOUT 等 errno 常量，用于判断错误类型。
#include "CommScheduler.h"                 // 引入通信调度器定义；虽然本文件直接使用不多，但 CommRequest 依赖相关类型。
#include "CommRequest.h"                   // 引入 CommRequest 类声明。

void CommRequest::handle(int state, int error) // handle(state, error)：通信请求完成或失败后的统一处理函数。
{                                             // 输入参数 state：通信层返回的状态，比如成功、错误、停止等。
	                                          // 输入参数 error：具体错误码；如果 state 是错误，error 表示错误原因。
	this->state = state;                      // 把输入参数 state 保存到成员变量 this->state，供上层 task->get_state() 查询。
	this->error = error;                      // 把输入参数 error 保存到成员变量 this->error，供上层 task->get_error() 查询。

	if (error != ETIMEDOUT)                   // 如果错误不是超时错误，就不需要进一步分析超时阶段。
		this->timeout_reason = TOR_NOT_TIMEOUT; // 设置 timeout_reason 为“不是超时”。
	else if (!this->target)                   // 如果是超时，并且 target 为空，说明还没拿到通信目标。
		this->timeout_reason = TOR_WAIT_TIMEOUT; // 设置为等待连接或等待目标时超时。
	else if (!this->get_message_out())        // 如果 target 已经有了，但还没有输出消息，说明还处于连接建立阶段。
		this->timeout_reason = TOR_CONNECT_TIMEOUT; // 设置为连接超时。
	else                                      // 否则说明请求已经进入发送或接收阶段。
		this->timeout_reason = TOR_TRANSMIT_TIMEOUT; // 设置为传输超时，也就是发送或接收过程超时。

	this->subtask_done();                     // 通知 Workflow：当前通信任务已经完成，可以调用 done() 并调度后续任务。
}
