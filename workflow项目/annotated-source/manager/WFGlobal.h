/*
  注释版源码文件：WFGlobal.h

  原始文件位置：
  workflow-master/src/manager/WFGlobal.h

  本文件用途：
  WFGlobal 是 Workflow 的“全局资源入口”。

  你可以把它理解成：
  1. 保存全局配置，例如 poller 线程数、handler 线程数、DNS 线程数。
  2. 持有全局单例资源，例如 CommScheduler、Executor、RouteManager、DnsCache。
  3. 给任务工厂和底层任务提供统一入口，避免用户手动到处创建调度器。

  对随机图库项目的意义：
  - HTTP Server、Redis Task、MySQL Task 都会间接用到全局 scheduler。
  - DNS、连接复用、线程池参数都可以通过这里理解。
  - 面试讲高并发时，可以从这里切入“全局 poller + handler + compute 线程池”的配置。
*/

#ifndef _WFGLOBAL_H_                                           // 头文件保护宏，避免同一个头文件被重复包含。
#define _WFGLOBAL_H_                                           // 定义保护宏。

#if __cplusplus < 201100                                       // 检查 C++ 标准版本是否低于 C++11。
#error CPLUSPLUS VERSION required at least C++11. Please use "-std=c++11". // Workflow 至少需要 C++11。
#include <C++11_REQUIRED>                                      // 故意包含一个不存在的头，用于强化编译错误提示。
#endif                                                         // 结束 C++11 检查。

#include <openssl/ssl.h>                                       // SSL_CTX 类型，用于 HTTPS、rediss 等 TLS 连接。
#include <string>                                              // std::string，用于 scheme、队列名等字符串。
#include "CommScheduler.h"                                     // CommScheduler：网络通信调度器。
#include "DnsCache.h"                                          // DnsCache：DNS 缓存。
#include "RouteManager.h"                                      // RouteManager：地址到连接目标的路由缓存。
#include "Executor.h"                                          // Executor：计算任务线程池执行器。
#include "EndpointParams.h"                                    // EndpointParams：连接超时、最大连接数等参数。
#include "WFResourcePool.h"                                    // WFResourcePool：资源池，用于 DNS 等场景。
#include "WFNameService.h"                                     // WFNameService：名字服务策略入口。
#include "WFDnsResolver.h"                                     // WFDnsResolver：默认 DNS 解析策略。

/**
 * @file    WFGlobal.h
 * @brief   Workflow Global Settings & Workflow Global APIs
 */

/**
 * @brief   Workflow Library Global Setting
 * @details
 * If you want set different settings with default, please call WORKFLOW_library_init at the beginning of the process
*/
struct WFGlobalSettings                                        // WFGlobalSettings：Workflow 进程级全局配置结构体。
{
	struct EndpointParams endpoint_params;                      // endpoint_params：普通网络请求默认连接参数，例如连接超时、响应超时、最大连接数。
	struct EndpointParams dns_server_params;                    // dns_server_params：访问 DNS 服务器时使用的连接参数。
	unsigned int dns_ttl_default;                               // dns_ttl_default：DNS 成功解析后的默认缓存时间，单位秒。
	unsigned int dns_ttl_min;                                   // dns_ttl_min：DNS 失败或异常时的最小缓存时间，单位秒，避免频繁打爆 DNS。
	int dns_threads;                                            // dns_threads：DNS 解析线程数，通常用于阻塞型系统 DNS 或 DNS 相关任务。
	int poller_threads;                                         // poller_threads：poller 线程数，决定有多少事件循环线程监听 fd。
	int handler_threads;                                        // handler_threads：handler 线程数，处理网络事件完成后的回调和状态推进。
	int compute_threads;                                        // compute_threads：计算线程池线程数；小于 0 时由系统 CPU 数自动决定。
	int fio_max_events;                                         // fio_max_events：文件 IO 最大事件数；图片读写任务会间接相关。
	const char *resolv_conf_path;                               // resolv_conf_path：DNS 配置文件路径，Linux 通常是 /etc/resolv.conf。
	const char *hosts_path;                                     // hosts_path：hosts 文件路径，Linux 通常是 /etc/hosts。
};

/**
 * @brief   Default Workflow Library Global Settings
 */
static constexpr struct WFGlobalSettings GLOBAL_SETTINGS_DEFAULT = // GLOBAL_SETTINGS_DEFAULT：默认全局配置。
{
	.endpoint_params	=	ENDPOINT_PARAMS_DEFAULT,             // 普通请求使用默认 Endpoint 参数。
	.dns_server_params	=	ENDPOINT_PARAMS_DEFAULT,             // DNS 请求也先复用默认 Endpoint 参数。
	.dns_ttl_default	=	3600,                                // DNS 成功结果默认缓存 3600 秒。
	.dns_ttl_min		=	60,                                  // DNS 最小缓存 60 秒。
	.dns_threads		=	4,                                   // 默认 4 个 DNS 线程。
	.poller_threads		=	4,                                   // 默认 4 个 poller 线程，负责监听 socket 事件。
	.handler_threads	=	20,                                  // 默认 20 个 handler 线程，负责处理事件完成逻辑。
	.compute_threads	=	-1,                                  // -1 表示让系统根据 CPU 数决定计算线程数。
	.fio_max_events		=	4096,                                // 文件 IO 同时管理的最大事件数。
	.resolv_conf_path	=	"/etc/resolv.conf",                  // 默认 DNS 配置路径。
	.hosts_path			=	"/etc/hosts",                        // 默认 hosts 文件路径。
};

/**
 * @brief      Reset Workflow Library Global Setting
 * @param[in]  settings          custom settings pointer
*/
extern void WORKFLOW_library_init(const struct WFGlobalSettings *settings); // WORKFLOW_library_init(settings)：进程启动早期调用，用于覆盖默认全局配置。

/**
 * @brief   Workflow Global Management Class
 * @details Workflow Global APIs
 */
class WFGlobal                                                  // WFGlobal：全局管理类，提供静态方法访问全局对象。
{
public:
	/**
	 * @brief      register default port for one scheme string
	 * @param[in]  scheme           scheme string
	 * @param[in]  port             default port value
	 * @warning    No effect when scheme is "http"/"https"/"redis"/"rediss"/"mysql"/"kafka"
	 */
	static void register_scheme_port(const std::string& scheme,  // scheme：协议名，例如自定义协议 "foo"。
									 unsigned short port);       // port：该协议默认端口。
	/**
	 * @brief      get default port string for one scheme string
	 * @param[in]  scheme           scheme string
	 * @return     port string const pointer
	 * @retval     NULL             fail, scheme not found
	 * @retval     not NULL         success
	 */
	static const char *get_default_port(const std::string& scheme); // get_default_port(scheme)：按协议名获取默认端口字符串。
	/**
	 * @brief      get current global settings
	 * @return     current global settings const pointer
	 * @note       returnval never NULL
	 */
	static const struct WFGlobalSettings *get_global_settings()  // get_global_settings()：获取当前全局配置。
	{
		return &settings_;                                       // 返回静态成员 settings_ 的地址；生命周期覆盖整个进程。
	}

	static void set_global_settings(const struct WFGlobalSettings *settings) // set_global_settings(settings)：设置全局配置。
	{
		settings_ = *settings;                                    // 拷贝用户传入配置，避免只保存外部指针导致悬空。
	}

	static const char *get_error_string(int state, int error);    // get_error_string(state,error)：把任务状态和错误码转成人类可读字符串。

	static bool increase_handler_thread()                         // increase_handler_thread()：动态增加 handler 线程。
	{
		return WFGlobal::get_scheduler()->increase_handler_thread() == 0; // 通过全局 scheduler 增加线程，返回是否成功。
	}

	static bool decrease_handler_thread()                         // decrease_handler_thread()：动态减少 handler 线程。
	{
		return WFGlobal::get_scheduler()->decrease_handler_thread() == 0; // 通过全局 scheduler 减少线程，返回是否成功。
	}

	static bool increase_compute_thread()                         // increase_compute_thread()：动态增加计算线程。
	{
		return WFGlobal::get_compute_executor()->increase_thread() == 0; // 通过全局 compute executor 增加线程。
	}

	static bool decrease_compute_thread()                         // decrease_compute_thread()：动态减少计算线程。
	{
		return WFGlobal::get_compute_executor()->decrease_thread() == 0; // 通过全局 compute executor 减少线程。
	}

	// Internal usage only
public:
	static bool is_scheduler_created();                           // is_scheduler_created()：判断全局 CommScheduler 是否已经创建。
	static class CommScheduler *get_scheduler();                  // get_scheduler()：获取全局网络调度器，HTTP/Redis/MySQL 都会用到。
	static SSL_CTX *get_ssl_client_ctx();                         // get_ssl_client_ctx()：获取全局客户端 SSL_CTX，用于 HTTPS/rediss。
	static SSL_CTX *new_ssl_server_ctx();                         // new_ssl_server_ctx()：创建服务端 SSL_CTX，用于 HTTPS Server。
	static class ExecQueue *get_exec_queue(const std::string& queue_name); // get_exec_queue(queue_name)：获取命名计算队列。
	static class Executor *get_compute_executor();                // get_compute_executor()：获取全局计算线程池。
	static class IOService *get_io_service();                     // get_io_service()：获取全局文件 IO 服务。
	static class ExecQueue *get_dns_queue();                      // get_dns_queue()：获取 DNS 任务队列。
	static class Executor *get_dns_executor();                    // get_dns_executor()：获取 DNS 线程池执行器。
	static class WFDnsClient *get_dns_client();                   // get_dns_client()：获取 DNS 客户端。
	static class WFResourcePool *get_dns_respool();               // get_dns_respool()：获取 DNS 资源池。

	static class RouteManager *get_route_manager()                // get_route_manager()：获取全局路由管理器。
	{
		return &route_manager_;                                   // 返回静态 RouteManager，负责缓存目标连接对象。
	}

	static class DnsCache *get_dns_cache()                        // get_dns_cache()：获取全局 DNS 缓存。
	{
		return &dns_cache_;                                       // 返回静态 DnsCache。
	}

	static class WFDnsResolver *get_dns_resolver()                // get_dns_resolver()：获取默认 DNS 解析策略。
	{
		return &dns_resolver_;                                    // 返回静态 WFDnsResolver。
	}

	static class WFNameService *get_name_service()                // get_name_service()：获取名字服务管理器。
	{
		return &name_service_;                                    // 返回静态 WFNameService，内部持有默认策略和自定义策略。
	}

public:
	static int sync_operation_begin();                            // sync_operation_begin()：同步操作开始，内部用于保护全局资源生命周期。
	static void sync_operation_end(int cookie);                   // sync_operation_end(cookie)：同步操作结束，cookie 是 begin 返回的标记。

private:
	static struct WFGlobalSettings settings_;                     // settings_：当前全局配置。
	static RouteManager route_manager_;                           // route_manager_：全局路由缓存与连接目标管理器。
	static DnsCache dns_cache_;                                   // dns_cache_：全局 DNS 缓存。
	static WFDnsResolver dns_resolver_;                           // dns_resolver_：默认 DNS 解析策略对象。
	static WFNameService name_service_;                           // name_service_：全局名字服务入口。
};

#endif                                                          // 结束头文件保护。

