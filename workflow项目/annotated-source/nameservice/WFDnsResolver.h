/*
  注释版源码文件：WFDnsResolver.h

  原始文件位置：
  workflow-master/src/nameservice/WFDnsResolver.h

  本文件用途：
  WFDnsResolver 是 Workflow 默认的名字解析策略。

  主要职责：
  1. 从 WFNSParams 中读取 URL、host、port、连接参数。
  2. 判断是否需要 DNS。
  3. 使用 DNS 缓存、线程 DNS 或网络 DNS 获取 addrinfo。
  4. 把 addrinfo 交给 RouteManager，得到可调度的连接目标。

  对随机图库项目的意义：
  - 你的 Redis URL、MySQL URL 都会经过这里。
  - 如果使用 127.0.0.1，这里可能更快进入固定地址路径。
  - 如果使用域名，例如 mysql.internal，则要理解 DNS 缓存和异步解析流程。
*/

#ifndef _WFDNSRESOLVER_H_                                      // 头文件保护宏。
#define _WFDNSRESOLVER_H_                                      // 定义头文件保护宏。

#include <string>                                              // std::string，解析 host/port 时可能使用。
#include <functional>                                          // std::function，路由回调用。
#include "EndpointParams.h"                                    // EndpointParams：连接参数。
#include "WFNameService.h"                                     // WFRouterTask、WFNSPolicy、WFNSParams。

class WFResolverTask : public WFRouterTask                     // WFResolverTask：默认 DNS 解析任务，继承路由任务。
{
public:
	WFResolverTask(const struct WFNSParams *ns_params,           // ns_params：名字服务参数，包含 URL、SSL、固定连接等信息。
				   unsigned int dns_ttl_default,                 // dns_ttl_default：DNS 成功结果默认缓存时间。
				   unsigned int dns_ttl_min,                     // dns_ttl_min：DNS 最小缓存时间。
				   const struct EndpointParams *ep_params,       // ep_params：连接参数。
				   router_callback_t&& cb) :                    // cb：路由完成回调。
		WFRouterTask(std::move(cb)),                             // 先构造父类，把回调移动进去。
		ns_params_(*ns_params),                                  // 拷贝 ns_params 内容，避免外部参数生命周期结束。
		ep_params_(*ep_params)                                   // 拷贝 endpoint 参数，后续可能按 fixed_conn 修改。
	{
		if (ns_params_.fixed_conn)                               // 如果任务要求固定连接。
			ep_params_.max_connections = 1;                      // 把最大连接数限制为 1，保证固定连接语义。

		dns_ttl_default_ = dns_ttl_default;                      // 保存 DNS 成功默认 TTL。
		dns_ttl_min_ = dns_ttl_min;                              // 保存 DNS 最小 TTL。
		has_next_ = false;                                       // has_next_：是否还有后续 DNS/地址结果，初始为 false。
		in_guard_ = false;                                       // in_guard_：是否处于 guard 保护逻辑，初始为 false。
		msg_ = NULL;                                             // msg_：DNS 临时消息/输出指针，初始为空。
	}

	WFResolverTask(const struct WFNSParams *ns_params,           // 第二个构造函数：只传名字服务参数和回调。
				   router_callback_t&& cb) :                    // cb：路由完成回调。
		WFRouterTask(std::move(cb)),                             // 构造父类并保存回调。
		ns_params_(*ns_params)                                   // 拷贝名字服务参数。
	{
		if (ns_params_.fixed_conn)                               // 如果要求固定连接。
			ep_params_.max_connections = 1;                      // 限制最大连接数为 1。

		has_next_ = false;                                       // 初始化是否有后续结果。
		in_guard_ = false;                                       // 初始化 guard 状态。
		msg_ = NULL;                                             // 初始化 DNS 临时消息为空。
	}

protected:
	virtual void dispatch();                                     // dispatch()：启动 DNS 解析/路由任务，是任务真正开始执行的入口。
	virtual SubTask *done();                                     // done()：DNS 路由任务完成后推进串行流。

private:
	void thread_dns_callback(void *thrd_dns_task);               // thread_dns_callback(thrd_dns_task)：线程 DNS 任务完成后的回调。
	void dns_single_callback(void *net_dns_task);                 // dns_single_callback(net_dns_task)：单个网络 DNS 请求完成回调。
	static void dns_partial_callback(void *net_dns_task);         // dns_partial_callback(net_dns_task)：静态中间回调，用于从 C 风格回调转回对象方法。
	void dns_parallel_callback(const void *parallel);             // dns_parallel_callback(parallel)：并行 DNS 请求全部或部分完成后的回调。
	void dns_callback_internal(void *thrd_dns_output,             // thrd_dns_output：线程 DNS 输出结果。
							   unsigned int ttl_default,         // ttl_default：成功 TTL。
							   unsigned int ttl_min);            // ttl_min：最小 TTL。

	void request_dns();                                          // request_dns()：发起 DNS 请求，可能走线程 DNS 或网络 DNS。
	void task_callback();                                        // task_callback()：解析完成后统一调用上层 callback。

protected:
	struct WFNSParams ns_params_;                                // ns_params_：本次解析任务的参数副本。
	unsigned int dns_ttl_default_;                               // dns_ttl_default_：成功 DNS 缓存 TTL。
	unsigned int dns_ttl_min_;                                   // dns_ttl_min_：最小 DNS 缓存 TTL。
	struct EndpointParams ep_params_;                            // ep_params_：本次目标连接参数副本。

private:
	const char *host_;                                           // host_：待解析主机名，来自 URL。
	unsigned short port_;                                        // port_：目标端口，来自 URL 或默认端口。
	bool has_next_;                                              // has_next_：是否还有可尝试的下一个地址或 DNS 分支。
	bool in_guard_;                                              // in_guard_：是否处于防重入/保护区间。
	void *msg_;                                                  // msg_：保存 DNS 相关临时消息，具体类型在 .cc 中处理。
};

class WFDnsResolver : public WFNSPolicy                         // WFDnsResolver：默认名字服务策略，实现 WFNSPolicy。
{
public:
	virtual WFRouterTask *create_router_task(const struct WFNSParams *params, // params：创建 DNS 路由任务所需参数。
											 router_callback_t callback);     // callback：任务完成后通知网络任务继续执行。
};

#endif                                                          // 结束头文件保护。

