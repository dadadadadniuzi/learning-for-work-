/*
  注释版源码文件：WFTaskFactory.h

  原始文件位置：
  workflow-master/src/factory/WFTaskFactory.h

  本文件用途：
  WFTaskFactory 是用户最常接触的任务创建入口。

  对随机图库项目最重要的函数：
  1. create_http_task()：创建 HTTP 客户端任务。
  2. create_redis_task()：创建 Redis 异步任务。
  3. create_mysql_task()：创建 MySQL 异步任务。
  4. create_pread_task()/create_pwrite_task()：创建文件读写任务。
  5. create_timer_task()：创建定时器任务。

  面试理解：
  WFTaskFactory 的作用就是“把底层复杂的通信、文件 IO、线程池等能力，包装成用户能直接使用的 Task 对象”。
*/

#ifndef _WFTASKFACTORY_H_                                      // 头文件保护宏：防止重复 include。
#define _WFTASKFACTORY_H_                                      // 定义头文件保护宏。

#include <sys/types.h>                                         // 引入 off_t 等系统类型，文件 IO 任务会用到。
#include <sys/uio.h>                                           // 引入 struct iovec，preadv/pwritev 任务会用到。
#include <time.h>                                              // 引入 time_t，定时器任务会用到。
#include <utility>                                             // 引入 std::move。
#include <functional>                                          // 引入 std::function，用于各种任务 callback。
#include <openssl/ssl.h>                                       // 引入 SSL_CTX，HTTPS/TLS 任务会用到。
#include "URIParser.h"                                         // ParsedURI：解析 URL 后的结构。
#include "RedisMessage.h"                                      // RedisRequest/RedisResponse 协议对象。
#include "HttpMessage.h"                                       // HttpRequest/HttpResponse 协议对象。
#include "MySQLMessage.h"                                      // MySQLRequest/MySQLResponse 协议对象。
#include "DnsMessage.h"                                        // DnsRequest/DnsResponse 协议对象。
#include "Workflow.h"                                          // SeriesWork/ParallelWork/Workflow。
#include "WFTask.h"                                            // 各类任务基类和模板。
#include "WFGraphTask.h"                                       // DAG 图任务。
#include "EndpointParams.h"                                    // EndpointParams：连接数、超时等网络端点参数。

using WFHttpTask = WFNetworkTask<protocol::HttpRequest,
								 protocol::HttpResponse>;       // WFHttpTask：HTTP 网络任务，请求类型是 HttpRequest，响应类型是 HttpResponse。
using http_callback_t = std::function<void (WFHttpTask *)>;     // http_callback_t：HTTP 任务完成回调类型。

using WFRedisTask = WFNetworkTask<protocol::RedisRequest,
								  protocol::RedisResponse>;     // WFRedisTask：Redis 网络任务。
using redis_callback_t = std::function<void (WFRedisTask *)>;   // redis_callback_t：Redis 任务完成回调类型。

using WFMySQLTask = WFNetworkTask<protocol::MySQLRequest,
								  protocol::MySQLResponse>;     // WFMySQLTask：MySQL 网络任务。
using mysql_callback_t = std::function<void (WFMySQLTask *)>;   // mysql_callback_t：MySQL 任务完成回调类型。

using WFDnsTask = WFNetworkTask<protocol::DnsRequest,
								protocol::DnsResponse>;         // WFDnsTask：DNS 网络任务。
using dns_callback_t = std::function<void (WFDnsTask *)>;       // dns_callback_t：DNS 任务完成回调类型。

struct FileIOArgs                                               // FileIOArgs：普通 pread/pwrite 文件任务参数。
{
	int fd;                                                     // fd：文件描述符，表示要读写哪个打开的文件。
	void *buf;                                                  // buf：读写缓冲区；读任务写入这里，写任务从这里取数据。
	size_t count;                                               // count：要读或写的字节数。
	off_t offset;                                               // offset：文件偏移量，从哪里开始读写。
};

struct FileVIOArgs                                              // FileVIOArgs：preadv/pwritev 向量 IO 参数。
{
	int fd;                                                     // fd：文件描述符。
	const struct iovec *iov;                                    // iov：多个缓冲区组成的数组。
	int iovcnt;                                                 // iovcnt：iov 数组长度。
	off_t offset;                                               // offset：文件偏移量。
};

struct FileSyncArgs                                             // FileSyncArgs：fsync/fdatasync 参数。
{
	int fd;                                                     // fd：需要刷盘的文件描述符。
};

using WFFileIOTask = WFFileTask<struct FileIOArgs>;             // WFFileIOTask：普通文件读写任务。
using fio_callback_t = std::function<void (WFFileIOTask *)>;    // fio_callback_t：普通文件 IO 任务回调。

using WFFileVIOTask = WFFileTask<struct FileVIOArgs>;           // WFFileVIOTask：向量文件 IO 任务。
using fvio_callback_t = std::function<void (WFFileVIOTask *)>;  // fvio_callback_t：向量文件 IO 回调。

using WFFileSyncTask = WFFileTask<struct FileSyncArgs>;         // WFFileSyncTask：文件同步刷盘任务。
using fsync_callback_t = std::function<void (WFFileSyncTask *)>;// fsync_callback_t：刷盘任务回调。

using timer_callback_t = std::function<void (WFTimerTask *)>;   // timer_callback_t：定时器任务回调。
using counter_callback_t = std::function<void (WFCounterTask *)>;// counter_callback_t：计数器任务回调。
using mailbox_callback_t = std::function<void (WFMailboxTask *)>;// mailbox_callback_t：mailbox 任务回调。
using selector_callback_t = std::function<void (WFSelectorTask *)>;// selector_callback_t：selector 任务回调。
using graph_callback_t = std::function<void (WFGraphTask *)>;   // graph_callback_t：图任务回调。

using WFEmptyTask = WFGenericTask;                              // WFEmptyTask：空任务，常用于占位。
using WFDynamicTask = WFGenericTask;                            // WFDynamicTask：动态任务，运行时生成下一个任务。
using dynamic_create_t = std::function<SubTask *(WFDynamicTask *)>; // dynamic_create_t：动态创建任务函数类型。
using repeated_create_t = std::function<SubTask *(WFRepeaterTask *)>; // repeated_create_t：重复任务生成函数类型。
using repeater_callback_t = std::function<void (WFRepeaterTask *)>;   // repeater_callback_t：重复任务结束回调。
using module_callback_t = std::function<void (const WFModuleTask *)>; // module_callback_t：模块任务回调。

class WFTaskFactory                                             // WFTaskFactory：任务工厂类，全部是静态方法。
{
public:
	static WFHttpTask *create_http_task(const std::string& url, // url：HTTP/HTTPS 请求 URL。
										int redirect_max,       // redirect_max：最大重定向次数。
										int retry_max,          // retry_max：最大重试次数。
										http_callback_t callback); // callback：HTTP 请求完成回调。

	static WFHttpTask *create_http_task(const ParsedURI& uri,   // uri：已经解析好的 URI。
										int redirect_max,       // redirect_max：最大重定向次数。
										int retry_max,          // retry_max：最大重试次数。
										http_callback_t callback); // callback：完成回调。

	static WFHttpTask *create_http_task(const std::string& url, // url：目标 URL。
										const std::string& proxy_url, // proxy_url：代理服务器 URL。
										int redirect_max,       // redirect_max：最大重定向次数。
										int retry_max,          // retry_max：最大重试次数。
										http_callback_t callback); // callback：完成回调。

	static WFHttpTask *create_http_task(const ParsedURI& uri,   // uri：目标 URI。
										const ParsedURI& proxy_uri, // proxy_uri：代理 URI。
										int redirect_max,       // redirect_max：最大重定向次数。
										int retry_max,          // retry_max：最大重试次数。
										http_callback_t callback); // callback：完成回调。

	static WFRedisTask *create_redis_task(const std::string& url, // url：Redis URL，如 redis://127.0.0.1:6379/0。
										  int retry_max,          // retry_max：最大重试次数。
										  redis_callback_t callback); // callback：Redis 命令完成回调。

	static WFRedisTask *create_redis_task(const ParsedURI& uri, // uri：解析后的 Redis URI。
										  int retry_max,        // retry_max：最大重试次数。
										  redis_callback_t callback); // callback：完成回调。

	static WFMySQLTask *create_mysql_task(const std::string& url, // url：MySQL URL，包含用户、密码、主机、库名。
										  int retry_max,          // retry_max：最大重试次数。
										  mysql_callback_t callback); // callback：SQL 执行完成回调。

	static WFMySQLTask *create_mysql_task(const ParsedURI& uri, // uri：解析后的 MySQL URI。
										  int retry_max,        // retry_max：最大重试次数。
										  mysql_callback_t callback); // callback：完成回调。

	static WFDnsTask *create_dns_task(const std::string& url,   // url：DNS 请求 URL。
									  int retry_max,            // retry_max：最大重试次数。
									  dns_callback_t callback); // callback：DNS 任务完成回调。

	static WFDnsTask *create_dns_task(const ParsedURI& uri,     // uri：解析后的 DNS URI。
									  int retry_max,            // retry_max：最大重试次数。
									  dns_callback_t callback); // callback：完成回调。

public:
	static WFFileIOTask *create_pread_task(int fd,              // fd：文件描述符。
										   void *buf,           // buf：读入数据保存位置。
										   size_t count,        // count：读取字节数。
										   off_t offset,        // offset：读取起始偏移。
										   fio_callback_t callback); // callback：读完成回调。

	static WFFileIOTask *create_pwrite_task(int fd,             // fd：文件描述符。
											const void *buf,     // buf：要写出的数据。
											size_t count,        // count：写入字节数。
											off_t offset,        // offset：写入起始偏移。
											fio_callback_t callback); // callback：写完成回调。

	static WFFileVIOTask *create_preadv_task(int fd,            // fd：文件描述符。
											 const struct iovec *iov, // iov：多个读缓冲区。
											 int iovcnt,        // iovcnt：缓冲区数量。
											 off_t offset,      // offset：读取偏移。
											 fvio_callback_t callback); // callback：完成回调。

	static WFFileVIOTask *create_pwritev_task(int fd,           // fd：文件描述符。
											  const struct iovec *iov, // iov：多个写缓冲区。
											  int iovcnt,       // iovcnt：缓冲区数量。
											  off_t offset,     // offset：写入偏移。
											  fvio_callback_t callback); // callback：完成回调。

	static WFFileSyncTask *create_fsync_task(int fd,            // fd：需要 fsync 的文件描述符。
											 fsync_callback_t callback); // callback：fsync 完成回调。

	static WFFileSyncTask *create_fdatasync_task(int fd,        // fd：需要 fdatasync 的文件描述符。
												 fsync_callback_t callback); // callback：fdatasync 完成回调。

public:
	static WFFileIOTask *create_pread_task(const std::string& path, // path：文件路径，工厂内部会打开文件。
										   void *buf,           // buf：读缓冲区。
										   size_t count,        // count：读取字节数。
										   off_t offset,        // offset：读取偏移。
										   fio_callback_t callback); // callback：完成回调。

	static WFFileIOTask *create_pwrite_task(const std::string& path, // path：文件路径。
											const void *buf,     // buf：写入数据。
											size_t count,        // count：写入字节数。
											off_t offset,        // offset：写入偏移。
											fio_callback_t callback); // callback：完成回调。

	static WFFileVIOTask *create_preadv_task(const std::string& path, // path：文件路径。
											 const struct iovec *iov, // iov：多个读缓冲区。
											 int iovcnt,        // iovcnt：缓冲区数量。
											 off_t offset,      // offset：读取偏移。
											 fvio_callback_t callback); // callback：完成回调。

	static WFFileVIOTask *create_pwritev_task(const std::string& path, // path：文件路径。
											  const struct iovec *iov, // iov：多个写缓冲区。
											  int iovcnt,       // iovcnt：缓冲区数量。
											  off_t offset,     // offset：写入偏移。
											  fvio_callback_t callback); // callback：完成回调。

public:
	static WFTimerTask *create_timer_task(time_t seconds,       // seconds：定时秒数。
										  long nanoseconds,     // nanoseconds：额外纳秒数。
										  timer_callback_t callback); // callback：定时器触发回调。

	static WFTimerTask *create_timer_task(const std::string& timer_name, // timer_name：命名定时器名称。
										  time_t seconds,       // seconds：秒数。
										  long nanoseconds,     // nanoseconds：纳秒数。
										  timer_callback_t callback); // callback：触发回调。

	static int cancel_by_name(const std::string& timer_name)    // cancel_by_name(timer_name)：取消某名称下所有定时器。
	{
		return WFTaskFactory::cancel_by_name(timer_name, (size_t)-1); // 调用重载版本，max=-1 表示尽可能全部取消。
	}

	static int cancel_by_name(const std::string& timer_name, size_t max); // 最多取消 max 个同名定时器。

	static WFTimerTask *create_timer_task(timer_callback_t callback); // 创建一个启动后立即取消/触发语义的特殊定时器。

	static WFTimerTask *create_timer_task(unsigned int microseconds, // microseconds：微秒数，旧接口。
										  timer_callback_t callback); // callback：触发回调。

public:
	static WFCounterTask *create_counter_task(unsigned int target_value, // target_value：目标计数值。
											  counter_callback_t callback) // callback：计数完成回调。
	{
		return new WFCounterTask(target_value, std::move(callback)); // 直接 new 计数器任务返回。
	}

	static WFCounterTask *create_counter_task(const std::string& counter_name, // counter_name：命名计数器名称。
											  unsigned int target_value, // target_value：目标计数。
											  counter_callback_t callback); // callback：完成回调。

	static int count_by_name(const std::string& counter_name)   // count_by_name(counter_name)：对同名计数器计数 1 次。
	{
		return WFTaskFactory::count_by_name(counter_name, 1);   // 调用重载版本，n=1。
	}

	static int count_by_name(const std::string& counter_name, unsigned int n); // 对同名计数器增加 n 次计数。

public:
	static WFMailboxTask *create_mailbox_task(void **mailbox,   // mailbox：消息保存位置。
											  mailbox_callback_t callback) // callback：消息到达回调。
	{
		return new WFMailboxTask(mailbox, std::move(callback)); // 创建 mailbox 任务。
	}

	static WFMailboxTask *create_mailbox_task(mailbox_callback_t callback) // 创建使用 user_data 作为 mailbox 的任务。
	{
		return new WFMailboxTask(std::move(callback));          // 创建并返回。
	}

	static WFSelectorTask *create_selector_task(size_t candidates, // candidates：候选者数量。
												selector_callback_t callback) // callback：选中后回调。
	{
		return new WFSelectorTask(candidates, std::move(callback)); // 创建 selector 任务。
	}

public:
	static WFConditional *create_conditional(SubTask *task, void **msgbuf) // task：条件满足后执行的任务；msgbuf：消息保存位置。
	{
		return new WFConditional(task, msgbuf);                 // 创建条件任务。
	}

	static WFConditional *create_conditional(SubTask *task)     // 创建默认把 user_data 当消息保存区的条件任务。
	{
		return new WFConditional(task);                         // 创建条件任务。
	}

public:
	template<class FUNC, class... ARGS>                         // FUNC：函数类型；ARGS：函数参数包。
	static WFGoTask *create_go_task(const std::string& queue_name, // queue_name：执行队列名称。
									FUNC&& func, ARGS&&... args); // func/args：要在线程池中执行的函数和参数。

	template<class FUNC, class... ARGS>
	static WFGoTask *create_timedgo_task(time_t seconds, long nanoseconds, // seconds/nanoseconds：运行时间限制。
										 const std::string& queue_name, // queue_name：执行队列。
										 FUNC&& func, ARGS&&... args); // func/args：函数和参数。

	template<class FUNC, class... ARGS>
	static WFGoTask *create_go_task(ExecQueue *queue, Executor *executor, // queue/executor：用户自定义执行队列和执行器。
									FUNC&& func, ARGS&&... args); // func/args：函数和参数。

	template<class FUNC, class... ARGS>
	static WFGoTask *create_timedgo_task(time_t seconds, long nanoseconds, // 时间限制。
										 ExecQueue *queue, Executor *executor, // 自定义队列和执行器。
										 FUNC&& func, ARGS&&... args); // 函数和参数。

	template<class FUNC, class... ARGS>
	static void reset_go_task(WFGoTask *task, FUNC&& func, ARGS&&... args); // 重置 go task 的执行函数。

public:
	static WFGraphTask *create_graph_task(graph_callback_t callback) // 创建 DAG 图任务。
	{
		return new WFGraphTask(std::move(callback));            // 创建并返回图任务。
	}

public:
	static WFEmptyTask *create_empty_task()                      // 创建空任务。
	{
		return new WFEmptyTask;                                 // 空任务启动后立即完成。
	}

	static WFDynamicTask *create_dynamic_task(dynamic_create_t create); // 创建动态任务，运行时由 create 生成下一任务。

	static WFRepeaterTask *create_repeater_task(repeated_create_t create, // create：每轮生成任务。
												repeater_callback_t callback) // callback：重复结束回调。
	{
		return new WFRepeaterTask(std::move(create), std::move(callback)); // 创建 repeater 任务。
	}

public:
	static WFModuleTask *create_module_task(SubTask *first,     // first：模块内部第一个任务。
											module_callback_t callback) // callback：模块完成回调。
	{
		return new WFModuleTask(first, std::move(callback));    // 创建模块任务。
	}

	static WFModuleTask *create_module_task(SubTask *first, SubTask *last, // first/last：模块内部首尾任务。
											module_callback_t callback) // callback：完成回调。
	{
		WFModuleTask *task = new WFModuleTask(first, std::move(callback)); // 创建模块任务。
		task->sub_series()->set_last_task(last);             // 设置模块内部子串行流的最后任务。
		return task;                                         // 返回模块任务。
	}
};

template<class REQ, class RESP>
class WFNetworkTaskFactory                                    // WFNetworkTaskFactory：更底层的网络任务工厂模板。
{
private:
	using T = WFNetworkTask<REQ, RESP>;                       // T：当前请求/响应类型对应的网络任务类型。

public:
	static T *create_client_task(enum TransportType type,       // type：传输类型，如 TCP/UDP/SCTP。
								 const std::string& host,       // host：目标主机。
								 unsigned short port,           // port：目标端口。
								 int retry_max,                 // retry_max：最大重试次数。
								 std::function<void (T *)> callback); // callback：完成回调。

	static T *create_client_task(enum TransportType type,
								 const std::string& url,        // url：目标 URL。
								 int retry_max,
								 std::function<void (T *)> callback);

	static T *create_client_task(enum TransportType type,
								 const ParsedURI& uri,          // uri：解析后的 URI。
								 int retry_max,
								 std::function<void (T *)> callback);

	static T *create_client_task(enum TransportType type,
								 const struct sockaddr *addr,   // addr：目标 socket 地址。
								 socklen_t addrlen,             // addrlen：地址长度。
								 int retry_max,
								 std::function<void (T *)> callback);

	static T *create_client_task(enum TransportType type,
								 const struct sockaddr *addr,
								 socklen_t addrlen,
								 SSL_CTX *ssl_ctx,              // ssl_ctx：SSL 上下文，HTTPS/TLS 场景使用。
								 int retry_max,
								 std::function<void (T *)> callback);
public:
	static T *create_server_task(CommService *service,          // service：服务端监听对象。
								 std::function<void (T *)>& process); // process：服务端用户处理函数。
};

template<class INPUT, class OUTPUT>
class WFThreadTaskFactory                                     // WFThreadTaskFactory：线程池任务工厂模板。
{
private:
	using T = WFThreadTask<INPUT, OUTPUT>;                     // T：当前输入/输出类型对应的线程任务类型。

public:
	static T *create_thread_task(const std::string& queue_name, // queue_name：线程池队列名。
								std::function<void (INPUT *, OUTPUT *)> routine, // routine：实际计算逻辑。
								std::function<void (T *)> callback); // callback：计算完成回调。

	static T *create_thread_task(time_t seconds, long nanoseconds, // seconds/nanoseconds：运行时间限制。
								const std::string& queue_name,
								std::function<void (INPUT *, OUTPUT *)> routine,
								std::function<void (T *)> callback);

public:
	static T *create_thread_task(ExecQueue *queue, Executor *executor, // 自定义队列和执行器。
								std::function<void (INPUT *, OUTPUT *)> routine,
								std::function<void (T *)> callback);

	static T *create_thread_task(time_t seconds, long nanoseconds,
								ExecQueue *queue, Executor *executor,
								std::function<void (INPUT *, OUTPUT *)> routine,
								std::function<void (T *)> callback);
};

#include "WFTaskFactory.inl"                                  // 引入模板实现；模板函数必须在头文件可见。

#endif                                                        // 结束头文件保护宏。
