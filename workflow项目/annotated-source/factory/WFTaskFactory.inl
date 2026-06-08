/*
  注释版源码文件：WFTaskFactory.inl

  原始文件位置：
  workflow-master/src/factory/WFTaskFactory.inl

  本文件用途：
  这里主要实现模板工厂：
  1. 动态任务 WFDynamicTask。
  2. 复杂客户端网络任务 WFComplexClientTask。
  3. WFNetworkTaskFactory 的 create_client_task/create_server_task。
  4. GoTask / ThreadTask 工厂。

  对随机图库项目来说，最重要的是：
  WFTaskFactory::create_redis_task()
  WFTaskFactory::create_mysql_task()
  这些高级函数最终会走到 WFNetworkTaskFactory / WFComplexClientTask 这套模板逻辑。
*/

#include <sys/types.h>                         // 系统类型。
#include <sys/socket.h>                        // sockaddr/socklen_t。
#include <errno.h>                             // errno 错误码。
#include <time.h>                              // time_t。
#include <netdb.h>                             // addrinfo。
#include <stdio.h>                             // sprintf。
#include <string>                              // std::string。
#include <functional>                          // std::function/std::bind。
#include <utility>                             // std::move/std::forward。
#include <atomic>                              // std::atomic。
#include <openssl/ssl.h>                       // SSL_CTX。
#include "WFGlobal.h"                          // 全局 scheduler、route manager、DNS resolver、线程池入口。
#include "Workflow.h"                          // SeriesWork 和任务编排。
#include "WFTask.h"                            // 任务基类。
#include "RouteManager.h"                      // 路由结果和通信目标缓存。
#include "URIParser.h"                         // URL 解析。
#include "WFTaskError.h"                       // Workflow 任务错误码。
#include "EndpointParams.h"                    // 端点参数。
#include "WFNameService.h"                     // 名字服务和路由策略。

class __WFDynamicTask : public WFDynamicTask    // __WFDynamicTask：动态任务的内部实现类。
{
protected:
	virtual void dispatch()                     // dispatch()：启动动态任务。
	{
		series_of(this)->push_front(this->create(this)); // 调用 create 生成一个新任务，并插到当前串行流队头。
		this->WFDynamicTask::dispatch();        // 调用基类 dispatch，立即 subtask_done()，让新任务继续执行。
	}

protected:
	std::function<SubTask *(WFDynamicTask *)> create; // create：用户提供的“运行时创建下一个任务”的函数。

public:
	__WFDynamicTask(std::function<SubTask *(WFDynamicTask *)>&& create) : // 构造函数。
		create(std::move(create))              // 保存 create 函数。
	{
	}
};

inline WFDynamicTask *WFTaskFactory::create_dynamic_task(dynamic_create_t create) // 创建动态任务。
{
	return new __WFDynamicTask(std::move(create)); // new 内部实现对象并返回基类指针。
}

template<>
int WFTaskFactory::send_by_name(const std::string&, void *const *, size_t); // send_by_name 的 void* 数组特化声明。

template<typename T>
int WFTaskFactory::send_by_name(const std::string& mailbox_name, T *const msg[],
								size_t max)
{
	return WFTaskFactory::send_by_name(mailbox_name, (void *const *)msg, max); // 把 T* 数组转成 void* 数组复用实现。
}

template<>
int WFTaskFactory::signal_by_name(const std::string&, void *const *, size_t); // signal_by_name 的 void* 数组特化声明。

template<typename T>
int WFTaskFactory::signal_by_name(const std::string& cond_name, T *const msg[],
								  size_t max)
{
	return WFTaskFactory::signal_by_name(cond_name, (void *const *)msg, max); // 把 T* 数组转成 void* 数组复用实现。
}

template<class REQ, class RESP, typename CTX = bool>
class WFComplexClientTask : public WFClientTask<REQ, RESP> // WFComplexClientTask：复杂客户端任务模板，支持 URL、DNS、路由、重试、重定向。
{
protected:
	using task_callback_t = std::function<void (WFNetworkTask<REQ, RESP> *)>; // 任务完成回调类型别名。

public:
	WFComplexClientTask(int retry_max, task_callback_t&& cb): // 构造函数，retry_max 是最大重试次数。
		WFClientTask<REQ, RESP>(NULL, WFGlobal::get_scheduler(), std::move(cb)) // 初始 request_object 为空，scheduler 使用全局调度器。
	{
		type_ = TT_TCP;                         // type_：默认传输类型是 TCP。
		ssl_ctx_ = NULL;                        // ssl_ctx_：默认不使用自定义 SSL 上下文。
		fixed_addr_ = false;                    // fixed_addr_：默认不是固定地址，需要走 URI/DNS/路由。
		fixed_conn_ = false;                    // fixed_conn_：默认不是固定连接。
		retry_max_ = retry_max;                 // retry_max_：保存最大重试次数。
		retry_times_ = 0;                       // retry_times_：当前已经重试次数，初始为 0。
		redirect_ = false;                      // redirect_：当前是否处在重定向过程，初始为 false。
		ns_policy_ = NULL;                      // ns_policy_：名字服务策略，初始为空，route() 时再获取。
		router_task_ = NULL;                    // router_task_：路由任务指针，初始为空。
	}

protected:
	virtual bool init_success() { return true; } // init_success()：子类可覆盖，用于初始化成功后的附加检查。
	virtual void init_failed() {}                // init_failed()：子类可覆盖，用于初始化失败时清理或标记。
	virtual bool check_request() { return true; } // check_request()：子类可覆盖，用于发送前检查请求是否合法。
	virtual WFRouterTask *route();               // route()：创建路由任务，负责 DNS/Upstream/RouteManager。
	virtual bool finish_once() { return true; }  // finish_once()：一次请求完成后的检查，子类可用于 redirect 或协议逻辑。

public:
	void init(const ParsedURI& uri)              // init(uri)：用解析好的 URI 初始化任务。
	{
		uri_ = uri;                              // 拷贝 URI 到成员变量。
		init_with_uri();                         // 根据 URI 做端口、DNS 策略等初始化。
	}

	void init(ParsedURI&& uri)                   // init(uri&&)：移动版本，避免拷贝 URI 内部字符串。
	{
		uri_ = std::move(uri);                   // 移动保存 URI。
		init_with_uri();                         // 根据 URI 初始化。
	}

	void init(enum TransportType type,           // type：传输类型。
			  const struct sockaddr *addr,       // addr：固定目标地址。
			  socklen_t addrlen,                 // addrlen：地址长度。
			  const std::string& info);          // info：额外路由信息。

	void set_transport_type(enum TransportType type) { type_ = type; } // 设置传输类型。

	enum TransportType get_transport_type() const { return type_; } // 获取传输类型。

	void set_ssl_ctx(SSL_CTX *ssl_ctx) { ssl_ctx_ = ssl_ctx; } // 设置 SSL 上下文。

	virtual const ParsedURI *get_current_uri() const { return &uri_; } // 获取当前 URI。

	void set_redirect(const ParsedURI& uri)       // set_redirect(uri)：把任务切换到重定向 URI。
	{
		redirect_ = true;                         // 标记正在重定向。
		init(uri);                                // 用新 URI 重新初始化。
	}

	void set_redirect(enum TransportType type, const struct sockaddr *addr,
					  socklen_t addrlen, const std::string& info) // 固定地址重定向。
	{
		redirect_ = true;                         // 标记重定向。
		init(type, addr, addrlen, info);          // 用固定地址初始化。
	}

	bool is_fixed_addr() const { return this->fixed_addr_; } // 是否固定地址。

	bool is_fixed_conn() const { return this->fixed_conn_; } // 是否固定连接。

protected:
	void set_fixed_addr(int fixed) { this->fixed_addr_ = fixed; } // 设置 fixed_addr_。

	void set_fixed_conn(int fixed) { this->fixed_conn_ = fixed; } // 设置 fixed_conn_。

	void set_info(const std::string& info) { info_.assign(info); } // 设置路由附加信息。

	void set_info(const char *info) { info_.assign(info); }        // C 字符串版本。

protected:
	virtual void dispatch();                       // dispatch()：复杂客户端任务真正的启动逻辑。
	virtual SubTask *done();                       // done()：网络请求/路由任务完成后的收尾和重试逻辑。

	void clear_resp()                              // clear_resp()：清空响应对象，但保留 ProtocolMessage 头部移动状态。
	{
		protocol::ProtocolMessage head(std::move(this->resp)); // 把 resp 中的 ProtocolMessage 部分移动出来。
		this->resp.~RESP();                       // 显式析构当前响应对象。
		new(&this->resp) RESP;                    // 在原地址 placement new 一个新的响应对象。
		*(protocol::ProtocolMessage *)&this->resp = std::move(head); // 把协议头部状态移回新响应对象。
	}

	void disable_retry() { retry_times_ = retry_max_; } // 禁用重试：把已重试次数置为最大值。

protected:
	enum TransportType type_;                  // type_：传输类型。
	ParsedURI uri_;                            // uri_：当前请求 URI。
	std::string info_;                          // info_：路由附加信息。
	SSL_CTX *ssl_ctx_;                          // ssl_ctx_：SSL 上下文。
	bool fixed_addr_;                           // fixed_addr_：是否固定地址。
	bool fixed_conn_;                           // fixed_conn_：是否固定连接。
	bool redirect_;                             // redirect_：是否处于重定向。
	CTX ctx_;                                   // ctx_：子类自定义上下文。
	int retry_max_;                             // retry_max_：最大重试次数。
	int retry_times_;                           // retry_times_：当前重试次数。
	WFNSPolicy *ns_policy_;                     // ns_policy_：名字服务策略。
	WFRouterTask *router_task_;                 // router_task_：路由任务。
	RouteManager::RouteResult route_result_;    // route_result_：路由结果，里面包含 request_object。
	WFNSTracing tracing_;                       // tracing_：名字服务追踪上下文。

public:
	CTX *get_mutable_ctx() { return &ctx_; }     // get_mutable_ctx()：获取可修改上下文。

private:
	void clear_prev_state();                     // clear_prev_state()：清理上一次请求状态，用于重试/重定向。
	void init_with_uri();                        // init_with_uri()：根据 uri_ 初始化。
	bool set_port();                             // set_port()：确保 URI 有端口。
	void router_callback(void *t);               // router_callback(t)：路由任务完成回调。
	void switch_callback(void *t);               // switch_callback(t)：切换到用户 callback 或重定向。
};

template<class REQ, class RESP, typename CTX>
void WFComplexClientTask<REQ, RESP, CTX>::clear_prev_state()
{
	ns_policy_ = NULL;                           // 清空名字服务策略，下次重新选择。
	route_result_.clear();                       // 清空路由结果。
	if (tracing_.deleter)                        // 如果 tracing_ 有自定义释放器。
	{
		tracing_.deleter(tracing_.data);          // 释放 tracing_ 数据。
		tracing_.deleter = NULL;                  // 清空释放器。
	}
	tracing_.data = NULL;                         // 清空 tracing 数据指针。
	retry_times_ = 0;                             // 重试次数归零。
	this->state = WFT_STATE_UNDEFINED;            // 状态恢复未运行。
	this->error = 0;                              // 错误码清零。
	this->timeout_reason = TOR_NOT_TIMEOUT;       // 超时原因清空。
}

template<class REQ, class RESP, typename CTX>
void WFComplexClientTask<REQ, RESP, CTX>::init(enum TransportType type,
											   const struct sockaddr *addr,
											   socklen_t addrlen,
											   const std::string& info)
{
	if (redirect_)                                // 如果这是重定向初始化。
		clear_prev_state();                      // 清理旧状态。

	auto params = WFGlobal::get_global_settings()->endpoint_params; // 读取全局端点配置。
	struct addrinfo addrinfo = { };                // 构造临时 addrinfo。
	addrinfo.ai_family = addr->sa_family;          // 地址族来自 addr。
	addrinfo.ai_addr = (struct sockaddr *)addr;    // 地址指针。
	addrinfo.ai_addrlen = addrlen;                 // 地址长度。

	type_ = type;                                  // 保存传输类型。
	info_.assign(info);                            // 保存路由信息。
	params.use_tls_sni = false;                    // 固定地址场景不使用 TLS SNI 主机名。
	if (WFGlobal::get_route_manager()->get(type, &addrinfo, info_, &params,
										   "", ssl_ctx_, route_result_) < 0) // 直接向 RouteManager 获取路由目标。
	{
		this->state = WFT_STATE_SYS_ERROR;         // 路由失败，设置系统错误。
		this->error = errno;                       // 保存 errno。
	}
	else if (this->init_success())                 // 如果路由成功且子类初始化成功。
		return;                                    // 初始化成功。

	this->init_failed();                           // 否则调用失败钩子。
}

template<class REQ, class RESP, typename CTX>
bool WFComplexClientTask<REQ, RESP, CTX>::set_port()
{
	if (uri_.port)                                  // 如果 URI 里已经有端口。
	{
		int port = atoi(uri_.port);                 // 把端口字符串转为整数。

		if (port <= 0 || port > 65535)              // 检查端口合法范围。
		{
			this->state = WFT_STATE_TASK_ERROR;     // 端口非法是任务错误。
			this->error = WFT_ERR_URI_PORT_INVALID; // 设置具体错误码。
			return false;                           // 返回失败。
		}

		return true;                                // 端口存在且合法。
	}

	if (uri_.scheme)                                // 如果没有端口，但有 scheme。
	{
		const char *port_str = WFGlobal::get_default_port(uri_.scheme); // 按 scheme 获取默认端口，如 http->80。

		if (port_str)                               // 如果找到默认端口。
		{
			uri_.port = strdup(port_str);            // 复制默认端口到 URI。
			if (uri_.port)                           // 复制成功。
				return true;                         // 返回成功。

			this->state = WFT_STATE_SYS_ERROR;       // strdup 失败是系统错误。
			this->error = errno;                     // 保存 errno。
			return false;
		}
	}

	this->state = WFT_STATE_TASK_ERROR;             // 没有端口也找不到 scheme 默认端口。
	this->error = WFT_ERR_URI_SCHEME_INVALID;       // 设置 scheme 非法错误。
	return false;                                   // 返回失败。
}

template<class REQ, class RESP, typename CTX>
void WFComplexClientTask<REQ, RESP, CTX>::init_with_uri()
{
	if (redirect_)                                  // 如果是重定向。
	{
		clear_prev_state();                         // 清理旧状态。
		ns_policy_ = WFGlobal::get_dns_resolver();  // 重定向后默认走 DNS resolver。
	}

	if (uri_.state == URI_STATE_SUCCESS)            // URI 解析成功。
	{
		if (this->set_port())                       // 确保端口合法。
		{
			if (this->init_success())                // 子类初始化成功。
				return;                              // 整体初始化成功。
		}
	}
	else if (uri_.state == URI_STATE_ERROR)          // URI parser 自身遇到系统错误。
	{
		this->state = WFT_STATE_SYS_ERROR;           // 设置系统错误。
		this->error = uri_.error;                    // 保存 parser 错误。
	}
	else                                            // URI 解析失败但不是系统错误。
	{
		this->state = WFT_STATE_TASK_ERROR;          // 设置任务错误。
		this->error = WFT_ERR_URI_PARSE_FAILED;      // URI 解析失败。
	}

	this->init_failed();                             // 初始化失败钩子。
}

template<class REQ, class RESP, typename CTX>
WFRouterTask *WFComplexClientTask<REQ, RESP, CTX>::route()
{
	auto&& cb = std::bind(&WFComplexClientTask::router_callback,
						  this,
						  std::placeholders::_1);   // cb：路由任务完成后回调到当前对象。
	struct WFNSParams params = {                      // params：名字服务创建路由任务需要的参数。
		.type			=	type_,                    // 传输类型。
		.uri			=	uri_,                     // 当前 URI。
		.info			=	info_.c_str(),             // 附加信息。
		.ssl_ctx		=	ssl_ctx_,                 // SSL 上下文。
		.fixed_addr		=	fixed_addr_,              // 是否固定地址。
		.fixed_conn		=	fixed_conn_,              // 是否固定连接。
		.retry_times	=	retry_times_,             // 当前重试次数。
		.tracing		=	&tracing_,                // 追踪上下文。
	};

	if (!ns_policy_)                                  // 如果还没有名字服务策略。
	{
		WFNameService *ns = WFGlobal::get_name_service(); // 获取全局名字服务。
		ns_policy_ = ns->get_policy(uri_.host ? uri_.host : ""); // 按 host 获取策略，找不到通常走默认 DNS resolver。
	}

	return ns_policy_->create_router_task(&params, std::move(cb)); // 创建路由任务。
}

template<class REQ, class RESP, typename CTX>
void WFComplexClientTask<REQ, RESP, CTX>::router_callback(void *t)
{
	WFRouterTask *task = (WFRouterTask *)t;          // t 是完成的路由任务。

	this->state = task->get_state();                 // 保存路由任务状态。
	if (this->state == WFT_STATE_SUCCESS)            // 路由成功。
		route_result_ = std::move(*task->get_result()); // 移动保存路由结果。
	else if (this->state == WFT_STATE_UNDEFINED)     // 理论不应该发生。
	{
		this->state = WFT_STATE_SYS_ERROR;           // 强制转成系统错误。
		this->error = ENOSYS;                        // 设置功能未实现错误。
	}
	else                                            // 路由失败。
		this->error = task->get_error();             // 保存路由错误码。
}

template<class REQ, class RESP, typename CTX>
void WFComplexClientTask<REQ, RESP, CTX>::dispatch()
{
	switch (this->state)                             // 根据任务当前状态决定下一步。
	{
	case WFT_STATE_UNDEFINED:                        // 初始状态，说明还没路由/还没发送请求。
		if (this->check_request())                   // 先检查请求合法性。
		{
			if (this->route_result_.request_object)  // 如果已经有路由结果。
			{
	case WFT_STATE_SUCCESS:                          // 路由成功后也会落到这里继续发请求。
				this->set_request_object(route_result_.request_object); // 设置 CommRequest 的调度对象。
				this->WFClientTask<REQ, RESP>::dispatch(); // 调用客户端任务 dispatch，真正发起网络请求。
				return;                                // 请求已提交，等待异步完成。
			}

			router_task_ = this->route();             // 没有路由结果，则创建路由任务。
			series_of(this)->push_front(this);        // 把自己重新插到队头，等路由完成后继续 dispatch。
			series_of(this)->push_front(router_task_);// 把路由任务插到自己前面，先执行路由。
		}

	default:
		break;                                       // 检查失败或状态异常，会走到 subtask_done。
	}

	this->subtask_done();                            // 当前阶段结束，推进流程。
}

template<class REQ, class RESP, typename CTX>
void WFComplexClientTask<REQ, RESP, CTX>::switch_callback(void *t)
{
	if (!redirect_)                                  // 如果不是重定向。
	{
		if (this->state == WFT_STATE_SYS_ERROR && this->error < 0) // SSL 错误用负数传递。
		{
			this->state = WFT_STATE_SSL_ERROR;       // 转成 SSL 错误状态。
			this->error = -this->error;              // 错误码转正。
		}

		if (tracing_.deleter)                        // 如果有 tracing 释放器。
		{
			tracing_.deleter(tracing_.data);          // 释放 tracing 数据。
			tracing_.deleter = NULL;                  // 清空释放器。
		}

		if (this->callback)                          // 如果用户设置了回调。
			this->callback(this);                    // 调用用户回调。
	}

	if (redirect_)                                   // 如果需要重定向。
	{
		redirect_ = false;                           // 清掉重定向标记。
		clear_resp();                                // 清理旧响应。
		this->target = NULL;                         // 清掉旧目标连接。
		series_of(this)->push_front(this);           // 把自己重新放入队头，发起新请求。
	}
	else                                            // 不重定向。
		delete this;                                 // 删除客户端任务对象。
}

template<class REQ, class RESP, typename CTX>
SubTask *WFComplexClientTask<REQ, RESP, CTX>::done()
{
	SeriesWork *series = series_of(this);            // 找到所属串行流。

	if (router_task_)                                // 如果刚完成的是路由任务。
	{
		router_task_ = NULL;                         // 清空路由任务标记。
		return series->pop();                        // 继续执行队列里的自己。
	}

	bool is_user_request = this->finish_once();      // 处理一次网络请求完成后的子类逻辑。

	if (ns_policy_)                                  // 如果有名字服务策略。
	{
		if (this->state == WFT_STATE_SYS_ERROR ||
			this->state == WFT_STATE_DNS_ERROR)      // 网络或 DNS 失败。
		{
			ns_policy_->failed(&route_result_, &tracing_, this->target); // 通知策略当前目标失败。
		}
		else if (route_result_.request_object)       // 请求成功且有路由对象。
		{
			ns_policy_->success(&route_result_, &tracing_, this->target); // 通知策略当前目标可用。
		}
	}

	if (this->state == WFT_STATE_SUCCESS)            // 如果网络请求成功。
	{
		if (!is_user_request)                        // 如果子类认为还不是最终用户请求，例如 HTTP redirect。
			return this;                             // 返回自己，继续下一轮。
	}
	else if (this->state == WFT_STATE_SYS_ERROR)     // 如果系统错误。
	{
		if (retry_times_ < retry_max_)               // 如果还可以重试。
		{
			redirect_ = true;                        // 用 redirect_ 机制重新调度自己。
			if (ns_policy_)                          // 如果走名字服务。
				route_result_.clear();               // 清掉旧路由，下次重新路由。

			this->state = WFT_STATE_UNDEFINED;       // 状态重置为未运行。
			this->error = 0;                         // 错误码清空。
			this->timeout_reason = 0;                // 超时原因清空。
			retry_times_++;                          // 重试次数加 1。
		}
	}

	if (!this->target || !this->CommSession::get_connection()) // 如果当前可能还在调用者线程，没有真实连接上下文。
	{
		auto&& cb = std::bind(&WFComplexClientTask::switch_callback,
							  this,
							  std::placeholders::_1); // 创建一个 timer 回调用来切换到 handler 线程。
		WFTimerTask *timer = WFTaskFactory::create_timer_task(std::move(cb)); // 创建立即触发/取消语义 timer。
		series->push_front(timer);                    // 插入 timer，避免递归调用导致栈过深。
	}
	else
		this->switch_callback(NULL);                   // 有连接上下文时直接执行 callback/重定向逻辑。

	return series->pop();                              // 返回下一个任务。
}

template<class REQ, class RESP>
WFNetworkTask<REQ, RESP> *
WFNetworkTaskFactory<REQ, RESP>::create_client_task(enum TransportType type,
													const std::string& host,
													unsigned short port,
													int retry_max,
													std::function<void (WFNetworkTask<REQ, RESP> *)> callback)
{
	auto *task = new WFComplexClientTask<REQ, RESP>(retry_max, std::move(callback)); // 创建复杂客户端任务。
	ParsedURI uri;                                      // 临时构造 URI。
	char buf[32];                                       // buf：保存端口字符串。

	sprintf(buf, "%u", port);                           // 把端口转成字符串。
	uri.scheme = strdup("scheme");                      // 构造一个占位 scheme。
	uri.host = strdup(host.c_str());                    // 复制 host。
	uri.port = strdup(buf);                             // 复制 port。
	if (!uri.scheme || !uri.host || !uri.port)           // 任一 strdup 失败。
	{
		uri.state = URI_STATE_ERROR;                    // 标记 URI 系统错误。
		uri.error = errno;                              // 保存 errno。
	}
	else
		uri.state = URI_STATE_SUCCESS;                  // URI 构造成功。

	task->init(std::move(uri));                         // 用 URI 初始化任务。
	task->set_transport_type(type);                     // 设置传输类型。
	return task;                                        // 返回任务。
}

template<class REQ, class RESP>
WFNetworkTask<REQ, RESP> *
WFNetworkTaskFactory<REQ, RESP>::create_client_task(enum TransportType type,
													const std::string& url,
													int retry_max,
													std::function<void (WFNetworkTask<REQ, RESP> *)> callback)
{
	auto *task = new WFComplexClientTask<REQ, RESP>(retry_max, std::move(callback)); // 创建客户端任务。
	ParsedURI uri;                                      // 保存解析后的 URI。

	URIParser::parse(url, uri);                         // 解析 URL 字符串。
	task->init(std::move(uri));                         // 用解析结果初始化。
	task->set_transport_type(type);                     // 设置传输类型。
	return task;                                        // 返回任务。
}

template<class REQ, class RESP>
WFNetworkTask<REQ, RESP> *
WFNetworkTaskFactory<REQ, RESP>::create_client_task(enum TransportType type,
													const ParsedURI& uri,
													int retry_max,
													std::function<void (WFNetworkTask<REQ, RESP> *)> callback)
{
	auto *task = new WFComplexClientTask<REQ, RESP>(retry_max, std::move(callback)); // 创建任务。

	task->init(uri);                                    // 用已有 URI 初始化。
	task->set_transport_type(type);                     // 设置传输类型。
	return task;                                        // 返回任务。
}

template<class REQ, class RESP>
WFNetworkTask<REQ, RESP> *
WFNetworkTaskFactory<REQ, RESP>::create_client_task(enum TransportType type,
													const struct sockaddr *addr,
													socklen_t addrlen,
													int retry_max,
													std::function<void (WFNetworkTask<REQ, RESP> *)> callback)
{
	auto *task = new WFComplexClientTask<REQ, RESP>(retry_max, std::move(callback)); // 创建任务。

	task->init(type, addr, addrlen, "");               // 用固定 sockaddr 初始化，绕过普通 URL/DNS。
	return task;                                        // 返回任务。
}

template<class REQ, class RESP>
WFNetworkTask<REQ, RESP> *
WFNetworkTaskFactory<REQ, RESP>::create_client_task(enum TransportType type,
													const struct sockaddr *addr,
													socklen_t addrlen,
													SSL_CTX *ssl_ctx,
													int retry_max,
													std::function<void (WFNetworkTask<REQ, RESP> *)> callback)
{
	auto *task = new WFComplexClientTask<REQ, RESP>(retry_max, std::move(callback)); // 创建任务。

	task->set_ssl_ctx(ssl_ctx);                        // 设置 SSL 上下文。
	task->init(type, addr, addrlen, "");               // 用固定地址初始化。
	return task;                                        // 返回任务。
}

template<class REQ, class RESP>
WFNetworkTask<REQ, RESP> *
WFNetworkTaskFactory<REQ, RESP>::create_server_task(CommService *service,
				std::function<void (WFNetworkTask<REQ, RESP> *)>& process)
{
	return new WFServerTask<REQ, RESP>(service, WFGlobal::get_scheduler(), process); // 创建服务端任务并绑定全局 scheduler。
}

class WFServerTaskFactory                         // WFServerTaskFactory：服务端任务工厂，供 WFHttpServer 等使用。
{
public:
	static WFDnsTask *create_dns_task(CommService *service,
					std::function<void (WFDnsTask *)>& process); // 创建 DNS 服务端任务。

	static WFHttpTask *create_http_task(CommService *service,
					std::function<void (WFHttpTask *)>& process); // 创建 HTTP 服务端任务。

	static WFMySQLTask *create_mysql_task(CommService *service,
					std::function<void (WFMySQLTask *)>& process); // 创建 MySQL 服务端任务。
};

class __WFGoTask : public WFGoTask                 // __WFGoTask：GoTask 内部实现。
{
public:
	void set_go_func(std::function<void ()> func) { this->go = std::move(func); } // 设置要执行的函数。

protected:
	virtual void execute() { this->go(); }       // execute()：在线程池中执行 go 函数。

protected:
	std::function<void ()> go;                    // go：被绑定好的无参函数。

public:
	__WFGoTask(ExecQueue *queue, Executor *executor,
			   std::function<void ()>&& func) :
		WFGoTask(queue, executor),                // 初始化父类 ExecRequest。
		go(std::move(func))                       // 保存函数。
	{
	}
};

template<class FUNC, class... ARGS>
WFGoTask *WFTaskFactory::create_go_task(const std::string& queue_name,
										FUNC&& func, ARGS&&... args)
{
	auto&& tmp = std::bind(std::forward<FUNC>(func),
						   std::forward<ARGS>(args)...); // 绑定函数和参数，形成无参函数。
	return new __WFGoTask(WFGlobal::get_exec_queue(queue_name),
						  WFGlobal::get_compute_executor(),
						  std::move(tmp));               // 使用全局计算线程池创建 GoTask。
}

template<class FUNC, class... ARGS>
WFGoTask *WFTaskFactory::create_go_task(ExecQueue *queue, Executor *executor,
										FUNC&& func, ARGS&&... args)
{
	auto&& tmp = std::bind(std::forward<FUNC>(func),
						   std::forward<ARGS>(args)...); // 绑定函数和参数。
	return new __WFGoTask(queue, executor, std::move(tmp)); // 使用用户指定队列和执行器创建。
}

template<class FUNC, class... ARGS>
void WFTaskFactory::reset_go_task(WFGoTask *task, FUNC&& func, ARGS&&... args)
{
	auto&& tmp = std::bind(std::forward<FUNC>(func),
						   std::forward<ARGS>(args)...); // 重新绑定函数。
	((__WFGoTask *)task)->set_go_func(std::move(tmp));    // 修改内部 go 函数。
}

template<class INPUT, class OUTPUT>
class __WFThreadTask : public WFThreadTask<INPUT, OUTPUT> // __WFThreadTask：线程池任务内部实现。
{
protected:
	virtual void execute()                                 // execute()：在线程池线程中执行。
	{
		this->routine(&this->input, &this->output);         // 把 input/output 地址交给用户 routine。
	}

protected:
	std::function<void (INPUT *, OUTPUT *)> routine;        // routine：用户计算逻辑。

public:
	__WFThreadTask(ExecQueue *queue, Executor *executor,
				   std::function<void (INPUT *, OUTPUT *)>&& rt,
				   std::function<void (WFThreadTask<INPUT, OUTPUT> *)>&& cb) :
		WFThreadTask<INPUT, OUTPUT>(queue, executor, std::move(cb)), // 初始化父类。
		routine(std::move(rt))                         // 保存计算函数。
	{
	}
};

template<class INPUT, class OUTPUT>
WFThreadTask<INPUT, OUTPUT> *
WFThreadTaskFactory<INPUT, OUTPUT>::create_thread_task(const std::string& queue_name,
						std::function<void (INPUT *, OUTPUT *)> routine,
						std::function<void (WFThreadTask<INPUT, OUTPUT> *)> callback)
{
	return new __WFThreadTask<INPUT, OUTPUT>(WFGlobal::get_exec_queue(queue_name),
											 WFGlobal::get_compute_executor(),
											 std::move(routine),
											 std::move(callback)); // 使用全局计算执行器创建线程任务。
}

template<class INPUT, class OUTPUT>
WFThreadTask<INPUT, OUTPUT> *
WFThreadTaskFactory<INPUT, OUTPUT>::create_thread_task(ExecQueue *queue, Executor *executor,
						std::function<void (INPUT *, OUTPUT *)> routine,
						std::function<void (WFThreadTask<INPUT, OUTPUT> *)> callback)
{
	return new __WFThreadTask<INPUT, OUTPUT>(queue, executor,
											 std::move(routine),
											 std::move(callback)); // 使用用户指定执行器创建线程任务。
}

