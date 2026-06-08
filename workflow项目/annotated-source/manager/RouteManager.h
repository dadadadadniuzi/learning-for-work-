/*
  注释版源码文件：RouteManager.h

  原始文件位置：
  workflow-master/src/manager/RouteManager.h

  本文件用途：
  RouteManager 负责把“解析出来的地址 + 协议参数”变成可被 CommScheduler 使用的目标对象。

  关键理解：
  1. DNS 解析得到 addrinfo 之后，并不是每次都新建连接目标。
  2. RouteManager 会缓存 RouteTarget，里面维护连接数、SSL_CTX、目标状态。
  3. CommScheduler 后续会围绕 CommSchedTarget 做连接复用和最大连接数控制。

  对随机图库项目的意义：
  - 访问 Redis、MySQL 时，URL 先经过名字服务和 DNS，然后进入 RouteManager。
  - 高并发时不是每个请求都创建新连接，而是按 target 控制连接数量。
*/

#ifndef _ROUTEMANAGER_H_                                       // 头文件保护宏。
#define _ROUTEMANAGER_H_                                       // 定义头文件保护宏。

#include <sys/types.h>                                         // 系统基础类型。
#include <sys/socket.h>                                        // sockaddr、socklen_t。
#include <netdb.h>                                             // addrinfo，DNS 解析结果结构。
#include <string>                                              // std::string。
#include <mutex>                                               // std::mutex，保护路由缓存红黑树。
#include <openssl/ssl.h>                                       // SSL_CTX，用于 TLS 目标。
#include "rbtree.h"                                            // 红黑树实现，用于缓存 RouteTarget。
#include "WFConnection.h"                                      // WFConnection：Workflow 默认连接对象。
#include "EndpointParams.h"                                    // EndpointParams：目标连接参数。
#include "CommScheduler.h"                                     // CommSchedTarget、CommSchedObject 等调度对象。

class RouteManager                                             // RouteManager：路由管理器，负责 target 缓存和可用性通知。
{
public:
	class RouteResult                                           // RouteResult：路由结果，返回给上层网络任务。
	{
	public:
		void *cookie;                                           // cookie：RouteManager 内部缓存节点标识，后续可用于通知可用/不可用。
		CommSchedObject *request_object;                        // request_object：可提交给 CommScheduler 的调度对象，可能是 target 或 group。

	public:
		RouteResult(): cookie(NULL), request_object(NULL) { }    // 构造函数：默认没有路由结果。
		void clear() { cookie = NULL; request_object = NULL; }   // clear()：清空结果，防止上一次结果误用。
	};

	class RouteTarget : public CommSchedTarget                   // RouteTarget：一个具体目标地址，继承可调度目标。
	{
	public:
		int init(const struct sockaddr *addr, socklen_t addrlen, // addr/addrlen：目标 IP 地址与长度。
				 SSL_CTX *ssl_ctx,                              // ssl_ctx：TLS 上下文；普通 TCP 为 NULL。
				 int connect_timeout,                           // connect_timeout：TCP 连接超时。
				 int ssl_connect_timeout,                       // ssl_connect_timeout：TLS 握手超时。
				 int response_timeout,                          // response_timeout：响应超时。
				 size_t max_connections)                        // max_connections：该目标最大并发连接数。
		{
			int ret = this->CommSchedTarget::init(addr, addrlen, ssl_ctx, // 调用父类初始化目标地址、SSL、超时、连接数。
								connect_timeout, ssl_connect_timeout,
								response_timeout, max_connections);

			if (ret >= 0 && ssl_ctx)                             // 如果初始化成功，并且使用了 SSL_CTX。
				SSL_CTX_up_ref(ssl_ctx);                         // 增加 SSL_CTX 引用计数，避免外部释放后这里变成悬空指针。

			return ret;                                          // 返回初始化结果；0 或正数通常表示成功，负数表示失败。
		}

		void deinit()                                            // deinit()：释放 RouteTarget 持有的资源。
		{
			SSL_CTX *ssl_ctx = this->get_ssl_ctx();              // 先取出 SSL_CTX，因为父类 deinit 可能清理内部字段。

			this->CommSchedTarget::deinit();                     // 释放父类目标资源，包括地址、连接列表等。
			if (ssl_ctx)                                        // 如果这个目标持有 SSL_CTX 引用。
				SSL_CTX_free(ssl_ctx);                           // 减少 SSL_CTX 引用计数，与 init 中的 up_ref 对应。
		}

	public:
		int state;                                               // state：目标状态，可用于标记可用、不可用等路由状态。

	private:
		virtual WFConnection *new_connection(int connect_fd)      // new_connection(connect_fd)：连接建立后创建连接对象。
		{
			return new WFConnection;                             // 默认返回 Workflow 的 WFConnection。
		}

	public:
		RouteTarget() : state(0) { }                              // 构造函数：state 初始化为 0。
	};

public:
	int get(enum TransportType type,                             // type：传输类型，例如 TT_TCP、TT_TCP_SSL 等。
			const struct addrinfo *addrinfo,                     // addrinfo：DNS 或固定地址解析出的地址链表。
			const std::string& other_info,                       // other_info：额外区分信息，例如协议相关标记。
			const struct EndpointParams *ep_params,              // ep_params：连接超时、最大连接数等 endpoint 参数。
			const std::string& hostname, SSL_CTX *ssl_ctx,       // hostname：主机名；ssl_ctx：TLS 上下文。
			RouteResult& result);                                // result：输出路由结果，包含 cookie 和 request_object。

	RouteManager()                                               // RouteManager 构造函数。
	{
		cache_.rb_node = NULL;                                   // 初始化红黑树根节点为空。
	}

	~RouteManager();                                             // 析构函数：释放缓存中的 RouteTarget。

private:
	std::mutex mutex_;                                           // mutex_：保护 cache_，因为多个任务可能并发访问路由缓存。
	struct rb_root cache_;                                       // cache_：红黑树根节点，用来按地址和参数缓存目标。

public:
	static void notify_unavailable(void *cookie, CommTarget *target); // notify_unavailable(cookie,target)：通知某个目标不可用，供失败回调调用。
	static void notify_available(void *cookie, CommTarget *target);   // notify_available(cookie,target)：通知某个目标恢复可用，供成功回调调用。
};

#endif                                                          // 结束头文件保护。

