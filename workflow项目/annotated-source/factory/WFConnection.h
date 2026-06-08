/*
  注释版源码文件：WFConnection.h

  原始文件位置：
  workflow-master/src/factory/WFConnection.h

  本文件用途：
  WFConnection 是 Workflow 默认连接对象，继承自底层 CommConnection。

  关键理解：
  - 连接对象不仅代表 socket 连接本身，还允许用户挂载一个 context。
  - context 可以保存和这条长连接相关的状态，例如认证信息、连接级缓存、统计信息。
  - context 使用 atomic 指针，是为了支持并发场景下安全地测试并设置。

  对随机图库项目的意义：
  - 第一阶段你不一定需要直接操作 WFConnection。
  - 面试讲连接复用时，可以说明：RouteTarget 创建 WFConnection，连接可被 CommTarget 维护和复用。
*/

#ifndef _WFCONNECTION_H_                                       // 头文件保护宏。
#define _WFCONNECTION_H_                                       // 定义头文件保护宏。

#include <utility>                                             // std::move，用于移动 deleter。
#include <atomic>                                              // std::atomic，用于原子 context 指针。
#include <functional>                                          // std::function，用于保存 context 释放函数。
#include "Communicator.h"                                      // CommConnection 基类。

class WFConnection : public CommConnection                     // WFConnection：Workflow 默认通信连接对象。
{
public:
	void *get_context() const                                   // get_context()：获取连接上挂载的用户上下文。
	{
		return this->context;                                   // 返回 atomic<void*> 当前保存的指针值。
	}

	void set_context(void *context, std::function<void (void *)> deleter) // set_context(context,deleter)：设置上下文并指定释放函数。
	{
		this->context = context;                                // 保存用户上下文指针。
		this->deleter = std::move(deleter);                     // 移动保存释放函数，避免拷贝函数对象。
	}

	void set_context(void *context)                             // set_context(context)：只设置上下文，不设置释放函数。
	{
		this->context = context;                                // 保存用户上下文指针；释放责任由外部承担。
	}

	void *test_set_context(void *test_context, void *new_context, // test_context：期望当前值；new_context：要写入的新值。
						   std::function<void (void *)> deleter) // deleter：新 context 对应的释放函数。
	{
		if (this->context.compare_exchange_strong(test_context, new_context)) // 如果当前 context 等于 test_context，就原子替换为 new_context。
		{
			this->deleter = std::move(deleter);                 // 替换成功后保存释放函数。
			return new_context;                                 // 返回新值，表示设置成功。
		}

		return test_context;                                    // 替换失败时，test_context 会被更新为真实当前值，并返回它。
	}

	void *test_set_context(void *test_context, void *new_context) // test_set_context：不带 deleter 的原子条件设置。
	{
		if (this->context.compare_exchange_strong(test_context, new_context)) // 如果当前值符合预期，就替换。
			return new_context;                              // 返回新值表示成功。

		return test_context;                                 // 返回真实当前值表示失败。
	}

private:
	std::atomic<void *> context;                              // context：连接级用户数据指针，用 atomic 支持并发条件设置。
	std::function<void (void *)> deleter;                     // deleter：连接销毁时用于释放 context 的函数。

public:
	WFConnection() : context(NULL) { }                        // 构造函数：默认没有用户上下文。

protected:
	virtual ~WFConnection()                                   // 析构函数：protected，通常由框架内部释放连接对象。
	{
		if (this->deleter)                                    // 如果用户提供了释放函数。
			this->deleter(this->context);                     // 调用释放函数清理 context。
	}
};

#endif                                                       // 结束头文件保护。

