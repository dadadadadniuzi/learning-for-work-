/*
  注释版源码文件：WFNameService.h

  原始文件位置：
  workflow-master/src/nameservice/WFNameService.h

  本文件用途：
  WFNameService 是 Workflow 的“名字服务策略管理器”。

  名字服务是什么：
  - 用户写的是 URL，例如 redis://127.0.0.1:6379 或 mysql://host:3306/db。
  - 网络任务不能直接拿 URL 发包，需要先得到具体地址和连接目标。
  - WFNameService 负责选择一个策略，把 URL/host 解析成 RouteManager 可用的结果。

  默认策略：
  - WFDnsResolver 是默认策略，负责 DNS 解析、DNS 缓存、RouteManager 路由。

  对随机图库项目的意义：
  - HTTP Server 处理请求后，你创建 Redis/MySQL task。
  - Redis/MySQL task 的 URL 会经过 WFNameService。
  - 最终得到可复用连接目标，再进入 CommScheduler。
*/

#ifndef _WFNAMESERVICE_H_                                      // 头文件保护宏。
#define _WFNAMESERVICE_H_                                      // 定义头文件保护宏。

#include <pthread.h>                                           // pthread_rwlock_t，读写锁。
#include <functional>                                          // std::function，用于保存 callback。
#include <utility>                                             // std::move。
#include "rbtree.h"                                            // 红黑树，用于保存名字服务策略。
#include "Communicator.h"                                      // CommTarget 等底层通信目标。
#include "Workflow.h"                                          // SeriesWork、series_of 等任务编排。
#include "WFTask.h"                                            // WFGenericTask 等任务类型。
#include "RouteManager.h"                                      // RouteManager::RouteResult。
#include "URIParser.h"                                         // ParsedURI，URL 解析结果。
#include "EndpointParams.h"                                    // EndpointParams，连接参数。

class WFRouterTask : public WFGenericTask                      // WFRouterTask：名字解析/路由任务，属于 Workflow 任务体系。
{
public:
	RouteManager::RouteResult *get_result() { return &this->result; } // get_result()：返回路由结果，后续网络任务用它发起请求。

public:
	void set_state(int state) { this->state = state; }           // set_state(state)：设置任务状态，例如成功、系统错误等。
	void set_error(int error) { this->error = error; }           // set_error(error)：设置具体错误码。

protected:
	RouteManager::RouteResult result;                            // result：保存 RouteManager 输出的 cookie 和 request_object。
	std::function<void (WFRouterTask *)> callback;               // callback：路由任务完成后的用户/框架回调。

protected:
	virtual SubTask *done()                                      // done()：路由任务结束后被 subtask_done() 调用。
	{
		SeriesWork *series = series_of(this);                    // 通过当前任务找回所属串行流。

		if (this->callback)                                      // 如果上层设置了回调函数。
			this->callback(this);                                // 执行回调，让网络任务拿到解析/路由结果。

		delete this;                                             // 删除当前路由任务；它通常是框架临时创建的。
		return series->pop();                                    // 返回串行流中的下一个任务，继续推进任务链。
	}

public:
	WFRouterTask(std::function<void (WFRouterTask *)>&& cb) :    // 构造函数，输入参数 cb 是完成回调。
		callback(std::move(cb))                                  // 使用 move 保存回调，避免不必要拷贝。
	{
	}
};

class WFNSTracing                                              // WFNSTracing：名字服务追踪数据，供高级策略携带额外信息。
{
public:
	void *data;                                                  // data：外部自定义追踪数据指针。
	void (*deleter)(void *);                                     // deleter：data 的释放函数，避免策略不知道如何析构外部数据。

public:
	WFNSTracing()                                                // 构造函数：默认没有追踪数据。
	{
		this->data = NULL;                                       // data 初始化为空。
		this->deleter = NULL;                                    // deleter 初始化为空。
	}
};

struct WFNSParams                                               // WFNSParams：创建名字服务/路由任务需要的参数集合。
{
	enum TransportType type;                                     // type：传输类型，例如 TCP 或 SSL TCP。
	ParsedURI& uri;                                             // uri：解析后的 URL，里面有 scheme、host、port、path 等。
	const char *info;                                            // info：额外信息，可用于区分同一 host 下的不同服务或策略。
	SSL_CTX *ssl_ctx;                                           // ssl_ctx：TLS 上下文；非 TLS 连接为 NULL。
	bool fixed_addr;                                             // fixed_addr：是否固定地址，固定地址通常跳过部分 DNS 逻辑。
	bool fixed_conn;                                             // fixed_conn：是否固定连接；为 true 时常把最大连接数限制为 1。
	int retry_times;                                             // retry_times：网络任务允许重试次数，名字服务可能据此选择策略。
	WFNSTracing *tracing;                                       // tracing：追踪信息，用于服务发现、负载均衡等扩展策略。
};

using router_callback_t = std::function<void (WFRouterTask *)>; // router_callback_t：路由任务完成回调类型别名。

class WFNSPolicy                                                // WFNSPolicy：名字服务策略接口。
{
public:
	virtual WFRouterTask *create_router_task(const struct WFNSParams *params, // params：路由所需参数。
											 router_callback_t callback) = 0; // callback：路由完成回调；返回值是创建出的路由任务。

	virtual void success(RouteManager::RouteResult *result,       // result：本次成功请求使用的路由结果。
						 WFNSTracing *tracing,                   // tracing：追踪数据。
						 CommTarget *target)                     // target：实际通信目标。
	{
		RouteManager::notify_available(result->cookie, target);   // 成功时通知 RouteManager，该目标可用。
	}

	virtual void failed(RouteManager::RouteResult *result,        // result：本次失败请求使用的路由结果。
						WFNSTracing *tracing,                    // tracing：追踪数据。
						CommTarget *target)                      // target：实际通信目标，可能为空。
	{
		if (target)                                              // 只有确实拿到目标时，才通知不可用。
			RouteManager::notify_unavailable(result->cookie, target); // 失败时通知 RouteManager，该目标可能不可用。
	}

public:
	virtual ~WFNSPolicy() { }                                    // 虚析构函数，允许通过基类指针删除自定义策略。
};

class WFNameService                                             // WFNameService：名字服务策略注册表。
{
public:
	int add_policy(const char *name, WFNSPolicy *policy);         // add_policy(name,policy)：注册一个名字服务策略。
	WFNSPolicy *get_policy(const char *name);                    // get_policy(name)：根据名称获取策略。
	WFNSPolicy *del_policy(const char *name);                    // del_policy(name)：删除并返回策略指针。

public:
	WFNSPolicy *get_default_policy() const                       // get_default_policy()：获取默认策略。
	{
		return this->default_policy;                              // 返回 default_policy，默认通常是 WFDnsResolver。
	}

	void set_default_policy(WFNSPolicy *policy)                  // set_default_policy(policy)：设置默认策略。
	{
		this->default_policy = policy;                            // 保存策略指针；调用者需要保证生命周期。
	}

private:
	WFNSPolicy *default_policy;                                  // default_policy：没有指定策略时使用的名字服务策略。
	struct rb_root root;                                         // root：红黑树根节点，保存 name -> policy。
	pthread_rwlock_t rwlock;                                     // rwlock：读写锁；读多写少，适合策略查询。

private:
	struct WFNSPolicyEntry *get_policy_entry(const char *name);   // get_policy_entry(name)：内部函数，查找红黑树节点。

public:
	WFNameService(WFNSPolicy *default_policy) :                  // 构造函数，输入默认策略指针。
		rwlock(PTHREAD_RWLOCK_INITIALIZER)                       // 初始化 pthread 读写锁。
	{
		this->root.rb_node = NULL;                                // 策略红黑树初始化为空。
		this->default_policy = default_policy;                    // 保存默认策略。
	}

	virtual ~WFNameService();                                     // 析构函数：释放策略表相关资源。
};

#endif                                                          // 结束头文件保护。

