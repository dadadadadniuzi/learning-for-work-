/*
  注释版源码文件：Communicator.cc

  原始文件位置：
  workflow-master/src/kernel/Communicator.cc

  本文件用途：
  Communicator 是 Workflow 的底层异步通信引擎。

  这个学习版重点注释“高并发主线”，覆盖：
  1. Communicator 如何创建 mpoller 和 handler 线程池。
  2. 客户端请求如何优先复用 idle 连接，复用失败才新建非阻塞连接。
  3. 服务端如何 bind 监听 fd，并在有新连接时创建 session。
  4. send_message 如何编码协议消息，并把读写事件交给 mpoller。
  5. poller 事件如何进入 msgqueue，再由 handler 线程调用 session->handle()。

  说明：
  原始 Communicator.cc 有 2000 多行，包含 SSL、UDP、AIO、push、sleep 等大量分支。
  这里不是完整复制原文件，而是为了面试学习提炼出最重要的执行路径。
*/

#include <sys/types.h>                                         // 系统类型。
#include <sys/socket.h>                                        // socket、connect、sendto 等。
#include <sys/uio.h>                                           // writev、iovec。
#include <errno.h>                                             // errno。
#include <limits.h>                                            // 系统限制。
#include <time.h>                                              // clock_gettime。
#include <fcntl.h>                                             // fcntl、O_NONBLOCK。
#include <unistd.h>                                            // close、sysconf、dup。
#include <stddef.h>                                            // size_t。
#include <stdlib.h>                                            // malloc、free。
#include <string.h>                                            // memcpy。
#include <pthread.h>                                           // pthread mutex。
#include <openssl/ssl.h>                                       // SSL。
#include <openssl/bio.h>                                       // BIO。
#include "list.h"                                             // 双向链表。
#include "msgqueue.h"                                         // handler 线程消费的消息队列。
#include "thrdpool.h"                                         // handler 线程池。
#include "poller.h"                                           // 单 poller 事件模型。
#include "mpoller.h"                                          // 多 poller 分发。
#include "Communicator.h"                                     // Communicator/CommTarget/CommSession 定义。

/*
  CommConnEntry 是 Communicator 内部最核心的连接状态对象。

  一条 TCP 连接在 Workflow 内部不是裸 fd，而是：
  fd + 连接对象 + 当前 session + target + 状态机 + 引用计数 + SSL 信息。
*/
struct CommConnEntry
{
	struct list_head list;                                      // list：把连接挂到 idle_list 或 keep_alive_list。
	CommConnection *conn;                                      // conn：用户可见的连接对象，例如 WFConnection。
	long long seq;                                             // seq：同一连接上的 session 序号。
	int sockfd;                                                // sockfd：真实 socket fd。
#define CONN_STATE_CONNECTING	0                               // 正在非阻塞 connect。
#define CONN_STATE_CONNECTED	1                               // 已连接。
#define CONN_STATE_RECEIVING	2                               // 正在接收响应/请求。
#define CONN_STATE_SUCCESS		3                               // 本次消息接收成功。
#define CONN_STATE_IDLE			4                               // 连接空闲，可复用。
#define CONN_STATE_KEEPALIVE	5                               // 服务端 keep-alive 等待下一次请求。
#define CONN_STATE_CLOSING		6                               // 正在关闭。
#define CONN_STATE_ERROR		7                               // 连接发生错误。
	int state;                                                 // state：当前连接状态。
	int error;                                                 // error：连接错误码。
	int ref;                                                   // ref：引用计数，防止事件处理和释放并发冲突。
	struct iovec *write_iov;                                   // write_iov：异步写时保存尚未写完的 iovec。
	SSL *ssl;                                                  // ssl：SSL 连接对象；普通 TCP 为 NULL。
	CommSession *session;                                      // session：当前这条连接上正在处理的会话。
	CommTarget *target;                                        // target：连接所属远端目标。
	CommService *service;                                      // service：服务端连接所属服务；客户端连接为 NULL。
	mpoller_t *mpoller;                                        // mpoller：该 fd 注册到的多 poller 对象。
	pthread_mutex_t mutex;                                     // mutex：客户端 session 场景下保护连接状态。
};

static inline int __set_fd_nonblock(int fd)                    // __set_fd_nonblock(fd)：把 fd 设置为非阻塞。
{
	int flags = fcntl(fd, F_GETFL);                             // 读取当前 fd flags。

	if (flags >= 0)                                             // 读取成功。
		flags = fcntl(fd, F_SETFL, flags | O_NONBLOCK);         // 写回 flags，并加上 O_NONBLOCK。

	return flags;                                               // 返回 fcntl 结果。
}

static void __release_conn(struct CommConnEntry *entry)         // __release_conn(entry)：释放一条连接的所有资源。
{
	delete entry->conn;                                         // 删除连接对象，例如 WFConnection。
	if (!entry->service)                                        // 客户端连接才初始化了 entry->mutex。
		pthread_mutex_destroy(&entry->mutex);                    // 销毁连接互斥锁。

	if (entry->ssl)                                             // 如果是 SSL 连接。
	{
		free(SSL_get_app_data(entry->ssl));                      // 释放 SSL_writev 临时缓冲。
		SSL_free(entry->ssl);                                   // 释放 SSL 对象。
	}

	close(entry->sockfd);                                       // 关闭 socket fd。
	free(entry);                                                // 释放 CommConnEntry 内存。
}

int CommTarget::init(const struct sockaddr *addr, socklen_t addrlen, // addr/addrlen：目标地址。
					 int connect_timeout, int response_timeout)       // connect/response：连接与响应超时。
{
	int ret;                                                    // ret：pthread 初始化结果。

	this->addr = (struct sockaddr *)malloc(addrlen);             // 为目标地址分配内存。
	if (this->addr)                                             // 分配成功。
	{
		ret = pthread_mutex_init(&this->mutex, NULL);            // 初始化 target 锁。
		if (ret == 0)                                           // 锁初始化成功。
		{
			memcpy(this->addr, addr, addrlen);                   // 保存目标地址。
			this->addrlen = addrlen;                            // 保存地址长度。
			this->connect_timeout = connect_timeout;             // 保存连接超时。
			this->response_timeout = response_timeout;           // 保存响应超时。
			INIT_LIST_HEAD(&this->idle_list);                   // 初始化空闲连接链表。

			this->ssl_ctx = NULL;                               // 默认不是 SSL。
			this->ssl_connect_timeout = 0;                      // 默认 SSL 握手超时为 0。
			return 0;                                           // 初始化成功。
		}

		errno = ret;                                            // pthread 错误写入 errno。
		free(this->addr);                                       // 释放地址内存。
	}

	return -1;                                                  // 初始化失败。
}

void CommTarget::deinit()                                      // CommTarget::deinit：释放目标资源。
{
	pthread_mutex_destroy(&this->mutex);                         // 销毁 target 锁。
	free(this->addr);                                           // 释放目标地址。
}

int CommMessageIn::feedback(const void *buf, size_t size)       // feedback(buf,size)：在解析输入时直接向对端反馈数据。
{
	struct CommConnEntry *entry = this->entry;                   // entry：当前消息所属连接。
	const struct sockaddr *addr = NULL;                          // addr：UDP 服务端可能需要目标地址。
	socklen_t addrlen = 0;                                       // addrlen：目标地址长度。
	int ret;                                                     // ret：写入结果。

	if (!entry->ssl)                                            // 普通非 SSL 连接。
	{
		if (entry->service)                                     // 服务端场景。
			entry->target->get_addr(&addr, &addrlen);            // UDP 需要取对端地址，TCP sendto 可传 NULL。

		return sendto(entry->sockfd, buf, size, 0, addr, addrlen); // 直接发送反馈数据。
	}

	if (size == 0)                                              // SSL 写 0 字节没有意义。
		return 0;                                               // 直接返回成功。

	ret = SSL_write(entry->ssl, buf, size);                      // SSL 写数据。
	if (ret <= 0)                                               // SSL 写失败或需要重试。
	{
		ret = SSL_get_error(entry->ssl, ret);                    // 获取 SSL 错误类型。
		if (ret == SSL_ERROR_WANT_READ || ret == SSL_ERROR_WANT_WRITE) // 需要等读/写事件。
			errno = EAGAIN;                                     // 映射为 EAGAIN。
		else if (ret != SSL_ERROR_SYSCALL)                      // 非系统调用错误。
			errno = -ret;                                       // 用负 SSL 错误码放入 errno。

		ret = -1;                                               // 对外返回失败。
	}

	return ret;                                                 // 返回写入结果。
}

/*
  连接复用和超时：
  first_timeout/next_timeout 用来决定本次读写事件交给 poller 后等待多久。
  send_timeout、receive_timeout 是 session 级别；
  response_timeout 是 target 级别。
*/
inline int Communicator::first_timeout(CommSession *session)    // first_timeout(session)：计算第一次事件等待超时。
{
	int timeout = session->target->response_timeout;             // 默认使用 target 响应超时。

	if (timeout < 0 || (unsigned int)session->timeout <= (unsigned int)timeout) // 如果 session 超时更短。
	{
		timeout = session->timeout;                              // 使用 session 超时。
		session->timeout = 0;                                    // 标记总超时已经用完。
	}
	else                                                        // target 单次超时更短。
		clock_gettime(CLOCK_MONOTONIC, &session->begin_time);    // 记录开始时间，后续 next_timeout 计算剩余总时间。

	return timeout;                                             // 返回本次 poller 超时时间。
}

int Communicator::next_timeout(CommSession *session)            // next_timeout(session)：计算后续事件的剩余超时。
{
	int timeout = session->target->response_timeout;             // 默认每次等待 target 响应超时。
	struct timespec cur_time;                                    // cur_time：当前时间。
	int time_used, time_left;                                    // time_used/time_left：已用时间和剩余时间。

	if (session->timeout > 0)                                   // 如果 session 还有总超时约束。
	{
		clock_gettime(CLOCK_MONOTONIC, &cur_time);               // 获取当前时间。
		time_used = 1000 * (cur_time.tv_sec - session->begin_time.tv_sec) + // 计算已用毫秒。
					(cur_time.tv_nsec - session->begin_time.tv_nsec) / 1000000;
		time_left = session->timeout - time_used;                // 剩余总超时。
		if (time_left <= timeout)                               // 如果剩余总超时比单次响应超时更短。
		{
			timeout = time_left < 0 ? 0 : time_left;              // 使用剩余时间，负数则设为 0。
			session->timeout = 0;                                // 标记总超时耗尽。
		}
	}

	return timeout;                                             // 返回下一次 poller 超时。
}

int Communicator::first_timeout_send(CommSession *session)      // first_timeout_send：发送阶段第一次超时。
{
	session->timeout = session->send_timeout();                  // 从 session 取发送总超时。
	return Communicator::first_timeout(session);                 // 交给 first_timeout 统一计算。
}

int Communicator::first_timeout_recv(CommSession *session)      // first_timeout_recv：接收阶段第一次超时。
{
	session->timeout = session->receive_timeout();               // 从 session 取接收总超时。
	return Communicator::first_timeout(session);                 // 交给 first_timeout 统一计算。
}

#ifndef IOV_MAX
# define IOV_MAX	16                                         // 如果系统没定义 IOV_MAX，默认最多一次 writev 16 个。
#endif

int Communicator::send_message_sync(struct iovec vectors[], int cnt, // vectors/cnt：待发送 iovec 数组。
									struct CommConnEntry *entry)      // entry：当前连接。
{
	CommSession *session = entry->session;                      // session：当前会话。
	CommService *service;                                       // service：服务端场景使用。
	int timeout;                                                // timeout：后续读/keep-alive 超时。
	ssize_t n;                                                  // n：writev 写出的字节数。
	int i;                                                       // i：iovec 遍历下标。

	while (cnt > 0)                                             // 只要还有 iovec 没写完。
	{
		if (!entry->ssl)                                        // 普通 TCP。
		{
			n = writev(entry->sockfd, vectors, cnt <= IOV_MAX ? cnt : IOV_MAX); // 尝试同步写。
			if (n < 0)                                          // 写失败。
				return errno == EAGAIN ? cnt : -1;               // EAGAIN 表示需要异步继续写，其他错误失败。
		}
		else if (vectors->iov_len > 0)                          // SSL 场景。
		{
			/* 原始代码使用 __ssl_writev 把多个 iovec 合并后 SSL_write。 */
			return cnt;                                         // 学习版省略 SSL 细节，返回剩余 iovec 交给异步路径。
		}
		else
			n = 0;                                              // 空 iovec。

		for (i = 0; i < cnt; i++)                               // 根据写出的字节数推进 iovec。
		{
			if ((size_t)n >= vectors[i].iov_len)                 // 当前 iovec 全部写完。
				n -= vectors[i].iov_len;                         // 扣掉当前长度。
			else if (n > 0)                                      // 当前 iovec 只写了一部分。
			{
				vectors[i].iov_base = (char *)vectors[i].iov_base + n; // 指针后移已写字节。
				vectors[i].iov_len -= n;                         // 长度减掉已写字节。
				return cnt - i;                                  // 返回剩余 iovec 数量，后续异步继续。
			}
			else
				break;                                          // 没有更多写出字节。
		}

		vectors += i;                                           // 跳过已写完 iovec。
		cnt -= i;                                               // 剩余 iovec 数减少。
	}

	service = entry->service;                                   // 判断是否服务端连接。
	if (service)                                                // 服务端发送 response 后。
	{
		__sync_add_and_fetch(&entry->ref, 1);                    // 增加引用，避免 keep-alive 注册期间被释放。
		timeout = session->keep_alive_timeout();                 // 获取 keep-alive 超时。
		switch (timeout)                                        // 根据 keep-alive 策略处理连接。
		{
		default:                                                // timeout 非 0，可以保持连接。
			mpoller_set_timeout(entry->sockfd, timeout, this->mpoller); // 设置下一次请求等待超时。
			pthread_mutex_lock(&service->mutex);                 // 加服务锁。
			if (service->listen_fd >= 0)                         // 服务仍在监听。
			{
				entry->state = CONN_STATE_KEEPALIVE;             // 连接进入 keep-alive 状态。
				list_add(&entry->list, &service->keep_alive_list); // 加入服务 keep-alive 链表。
				entry = NULL;                                    // 表示连接继续保留。
			}

			pthread_mutex_unlock(&service->mutex);               // 解锁。
			if (entry)                                          // 如果服务已关闭，不能 keep-alive。
			{
		case 0:                                                  // keep_alive_timeout 为 0，直接关闭。
				mpoller_del(entry->sockfd, this->mpoller);       // 从 poller 删除 fd。
				entry->state = CONN_STATE_CLOSING;               // 标记关闭。
			}
		}
	}
	else                                                        // 客户端发送 request 后。
	{
		if (entry->state == CONN_STATE_IDLE)                     // 如果复用的是 idle 连接。
		{
			timeout = session->first_timeout();                  // 获取第一个超时。
			if (timeout == 0)                                   // 如果没有特殊 first timeout。
				timeout = Communicator::first_timeout_recv(session); // 进入接收响应超时。
			else                                                // 否则重置 session 超时标记。
			{
				session->timeout = -1;                           // 后续不再用总超时。
				session->begin_time.tv_sec = -1;                 // begin_time 标记无效。
				session->begin_time.tv_nsec = 0;
			}

			mpoller_set_timeout(entry->sockfd, timeout, this->mpoller); // 设置接收响应超时。
		}

		entry->state = CONN_STATE_RECEIVING;                     // 客户端进入等待响应状态。
	}

	return 0;                                                   // 全部同步写完。
}

int Communicator::send_message_async(struct iovec vectors[], int cnt, // 剩余未写完的 iovec。
									 struct CommConnEntry *entry)      // 当前连接。
{
	struct poller_data data;                                    // data：要注册到 mpoller 的写事件。
	int timeout;                                                // timeout：发送超时。
	int ret;                                                    // ret：mpoller_add/mod 结果。
	int i;                                                       // i：复制 iovec 下标。

	entry->write_iov = (struct iovec *)malloc(cnt * sizeof (struct iovec)); // 保存剩余 iovec，不能指向栈。
	if (entry->write_iov)                                       // 分配成功。
	{
		for (i = 0; i < cnt; i++)                                // 复制每个 iovec。
			entry->write_iov[i] = vectors[i];                    // 保存到 entry。
	}
	else
		return -1;                                              // 分配失败。

	data.operation = PD_OP_WRITE;                               // poller 操作类型：写事件。
	data.fd = entry->sockfd;                                    // fd：当前 socket。
	data.ssl = entry->ssl;                                      // ssl：SSL 对象。
	data.partial_written = Communicator::partial_written;        // partial_written：部分写入后的回调。
	data.context = entry;                                       // context：回调时能拿回连接对象。
	data.write_iov = entry->write_iov;                           // 待写 iovec。
	data.iovcnt = cnt;                                          // iovec 数量。
	timeout = Communicator::first_timeout_send(entry->session);  // 计算发送超时。
	if (entry->state == CONN_STATE_IDLE)                         // 如果是复用 idle 连接。
	{
		ret = mpoller_mod(&data, timeout, this->mpoller);        // 修改原来的读事件为写事件。
		if (ret < 0 && errno == ENOENT)                          // 如果 fd 不在 poller 里。
			entry->state = CONN_STATE_RECEIVING;                 // 交给后续错误路径处理。
	}
	else                                                        // 新连接或非 idle 连接。
	{
		ret = mpoller_add(&data, timeout, this->mpoller);        // 新增写事件。
		if (ret >= 0)                                           // 添加成功。
		{
			if (this->stop_flag)                                // 如果 Communicator 正在停止。
				mpoller_del(data.fd, this->mpoller);             // 立刻删除事件。
		}
	}

	if (ret < 0)                                                // 注册写事件失败。
	{
		free(entry->write_iov);                                 // 释放复制的 iovec。
		if (entry->state != CONN_STATE_RECEIVING)                // 如果不是可继续进入接收的特殊状态。
			return -1;                                          // 返回失败。
	}

	return 1;                                                   // 返回 1 表示已进入异步写。
}

#define ENCODE_IOV_MAX		2048                                // 协议 encode 最多输出 iovec 数。

int Communicator::send_message(struct CommConnEntry *entry)     // send_message(entry)：编码并发送当前 session 的输出消息。
{
	struct iovec vectors[ENCODE_IOV_MAX];                       // vectors：栈上 iovec 缓冲。
	struct iovec *end;                                          // end：iovec 末尾。
	int cnt;                                                    // cnt：协议编码出的 iovec 数量。

	cnt = entry->session->out->encode(vectors, ENCODE_IOV_MAX);  // 调用协议层 encode，例如 HTTP/Redis/MySQL。
	if ((unsigned int)cnt > ENCODE_IOV_MAX)                     // cnt < 0 或 cnt 超上限都进入异常处理。
	{
		if (cnt > ENCODE_IOV_MAX)                               // 真正超过上限。
			errno = EOVERFLOW;                                  // 设置溢出。
		return -1;                                             // 发送失败。
	}

	end = vectors + cnt;                                        // end 指向 iovec 末尾。
	cnt = this->send_message_sync(vectors, cnt, entry);          // 先尝试同步 writev，减少 poller 写事件开销。
	if (cnt <= 0)                                               // cnt == 0 全部写完；cnt < 0 出错。
		return cnt;                                             // 直接返回。

	return this->send_message_async(end - cnt, cnt, entry);      // 剩余部分注册异步写事件。
}

void Communicator::callback(struct poller_result *res, void *context) // poller 线程回调：收到 fd 事件结果。
{
	Communicator *comm = (Communicator *)context;                // context 是 Communicator 指针。
	msgqueue_put(res, comm->msgqueue);                           // 把 poller_result 放入消息队列，由 handler 线程处理。
}

int Communicator::create_handler_threads(size_t handler_threads) // create_handler_threads：创建 handler 线程池。
{
	struct thrdpool_task task = {                                // task：每个 handler 线程执行的任务。
		.routine	=	Communicator::handler_thread_routine,    // routine：handler 线程主循环。
		.context	=	this                                    // context：当前 Communicator。
	};
	size_t i;                                                    // i：线程任务投递计数。

	this->thrdpool = thrdpool_create(handler_threads, 0);        // 创建线程池。
	if (this->thrdpool)                                         // 创建成功。
	{
		for (i = 0; i < handler_threads; i++)                    // 给每个线程投递一个常驻 handler 任务。
		{
			if (thrdpool_schedule(&task, this->thrdpool) < 0)    // 投递失败。
				break;                                          // 停止。
		}

		if (i == handler_threads)                               // 所有 handler 任务都投递成功。
			return 0;                                           // 创建成功。

		msgqueue_set_nonblock(this->msgqueue);                  // 失败时把队列设为非阻塞，方便线程退出。
		thrdpool_destroy(NULL, this->thrdpool);                 // 销毁线程池。
	}

	return -1;                                                  // 创建失败。
}

int Communicator::create_poller(size_t poller_threads)          // create_poller：创建 mpoller 和消息队列。
{
	struct poller_params params = {                              // params：poller 参数。
		.max_open_files		=	(size_t)sysconf(_SC_OPEN_MAX),   // 最大 fd 数取系统 open files 限制。
		.callback			=	Communicator::callback,         // poller 事件完成后调用 callback。
		.context			=	this                            // callback 的 context 是当前 Communicator。
	};

	if ((ssize_t)params.max_open_files < 0)                      // sysconf 失败。
		return -1;                                             // 返回失败。

	if (params.max_open_files > 1024 * 1024)                    // 防止过大。
		params.max_open_files = 1024 * 1024;                    // 上限限制为 1048576。

	this->msgqueue = msgqueue_create(16 * 1024, sizeof (struct poller_result)); // 创建 poller_result 队列。
	if (this->msgqueue)                                         // 队列创建成功。
	{
		this->mpoller = mpoller_create(&params, poller_threads); // 创建多 poller。
		if (this->mpoller)                                      // mpoller 创建成功。
		{
			if (mpoller_start(this->mpoller) >= 0)               // 启动 poller 线程。
				return 0;                                       // 成功。

			mpoller_destroy(this->mpoller);                     // 启动失败，销毁 mpoller。
		}

		msgqueue_destroy(this->msgqueue);                       // 销毁消息队列。
	}

	return -1;                                                  // 创建失败。
}

int Communicator::init(size_t poller_threads, size_t handler_threads) // init：初始化通信引擎。
{
	if (poller_threads == 0)                                    // poller 线程数不能为 0。
	{
		errno = EINVAL;                                         // 参数错误。
		return -1;                                             // 初始化失败。
	}

	if (this->create_poller(poller_threads) >= 0)                // 先创建 poller 与 msgqueue。
	{
		if (this->create_handler_threads(handler_threads) >= 0)  // 再创建 handler 线程池。
		{
			this->event_handler = NULL;                         // 默认不使用自定义事件处理器。
			this->stop_flag = 0;                                // stop_flag 清零，表示运行中。
			return 0;                                           // 初始化成功。
		}

		mpoller_stop(this->mpoller);                            // handler 创建失败，停止 poller。
		mpoller_destroy(this->mpoller);                         // 销毁 mpoller。
		msgqueue_destroy(this->msgqueue);                       // 销毁队列。
	}

	return -1;                                                  // 初始化失败。
}

void Communicator::deinit()                                    // deinit：关闭通信引擎。
{
	this->stop_flag = 1;                                        // 标记停止，后续新事件会被删除。
	mpoller_stop(this->mpoller);                                // 停止 poller 线程。
	if (this->event_handler)                                    // 如果使用自定义事件处理器。
		this->event_handler->wait();                            // 等待其处理结束。

	msgqueue_set_nonblock(this->msgqueue);                      // 设置消息队列非阻塞，唤醒 handler 退出。
	thrdpool_destroy(NULL, this->thrdpool);                     // 销毁 handler 线程池。
	mpoller_destroy(this->mpoller);                             // 销毁 mpoller。
	msgqueue_destroy(this->msgqueue);                           // 销毁消息队列。
}

int Communicator::nonblock_connect(CommTarget *target)          // nonblock_connect(target)：创建非阻塞连接。
{
	int sockfd = target->create_connect_fd();                    // 让 target 创建 socket fd。

	if (sockfd >= 0)                                            // socket 创建成功。
	{
		if (__set_fd_nonblock(sockfd) >= 0)                      // 设置非阻塞。
		{
			if (connect(sockfd, target->addr, target->addrlen) >= 0 || // connect 立即成功。
				errno == EINPROGRESS)                            // 或正在进行，非阻塞 connect 的正常状态。
			{
				return sockfd;                                  // 返回 fd，后续交给 poller 等连接完成。
			}
		}

		close(sockfd);                                          // 设置非阻塞或 connect 失败，关闭 fd。
	}

	return -1;                                                  // 连接创建失败。
}

struct CommConnEntry *Communicator::launch_conn(CommSession *session, // session：本次请求会话。
												CommTarget *target)  // target：目标服务。
{
	struct CommConnEntry *entry;                                 // entry：连接状态对象。
	int sockfd;                                                  // sockfd：新建 socket。
	int ret;                                                     // ret：mutex 初始化结果。

	sockfd = Communicator::nonblock_connect(target);             // 创建非阻塞连接。
	if (sockfd >= 0)                                            // fd 创建成功。
	{
		entry = (struct CommConnEntry *)malloc(sizeof (struct CommConnEntry)); // 分配连接状态对象。
		if (entry)                                              // 分配成功。
		{
			ret = pthread_mutex_init(&entry->mutex, NULL);       // 初始化连接锁。
			if (ret == 0)                                       // 锁初始化成功。
			{
				entry->conn = target->new_connection(sockfd);    // 创建用户连接对象，例如 WFConnection。
				if (entry->conn)                                // 连接对象创建成功。
				{
					entry->seq = 0;                              // 连接内 session 序号从 0 开始。
					entry->mpoller = NULL;                       // 后面再绑定 mpoller。
					entry->service = NULL;                       // 客户端连接没有 service。
					entry->target = target;                      // 保存目标。
					entry->session = session;                    // 保存当前 session。
					entry->ssl = NULL;                           // 默认无 SSL。
					entry->sockfd = sockfd;                      // 保存 socket fd。
					entry->state = CONN_STATE_CONNECTING;        // 状态为正在连接。
					entry->ref = 1;                              // 初始引用计数为 1。
					return entry;                                // 返回新连接对象。
				}

				pthread_mutex_destroy(&entry->mutex);            // 连接对象创建失败，销毁锁。
			}
			else
				errno = ret;                                    // mutex 错误写 errno。

			free(entry);                                        // 释放 entry。
		}

		close(sockfd);                                          // 关闭 fd。
	}

	return NULL;                                                // 创建连接失败。
}

int Communicator::request_idle_conn(CommSession *session, CommTarget *target) // request_idle_conn：尝试复用空闲连接。
{
	struct CommConnEntry *entry;                                 // entry：取出的 idle 连接。
	struct list_head *pos;                                       // pos：链表节点。
	int ret = -1;                                                // ret：发送结果。

	while (1)                                                    // 循环是为了跳过已经失效的 idle 连接。
	{
		pthread_mutex_lock(&target->mutex);                      // 锁住 target idle_list。
		if (!list_empty(&target->idle_list))                     // 如果有 idle 连接。
		{
			pos = target->idle_list.next;                        // 取链表第一个 idle 连接。
			entry = list_entry(pos, struct CommConnEntry, list); // 还原 entry。
			list_del(pos);                                      // 从 idle_list 移除，表示即将使用。
			pthread_mutex_lock(&entry->mutex);                   // 锁住 entry，保护状态。
		}
		else
			entry = NULL;                                       // 没有 idle 连接。

		pthread_mutex_unlock(&target->mutex);                    // 解 target 锁。
		if (!entry)                                             // 没有可复用连接。
		{
			errno = ENOENT;                                     // 设置不存在。
			return -1;                                         // 返回失败，调用方会新建连接。
		}

		if (mpoller_set_timeout(entry->sockfd, -1, this->mpoller) >= 0) // 取消 idle 连接上的超时。
			break;                                              // 成功取得可用 idle 连接。

		entry->state = CONN_STATE_CLOSING;                       // 如果设置超时失败，说明 fd 可能已失效。
		pthread_mutex_unlock(&entry->mutex);                     // 解锁后继续尝试下一个 idle。
	}

	entry->session = session;                                   // 把连接绑定到新 session。
	session->conn = entry->conn;                                // session 记录连接对象。
	session->seq = entry->seq++;                                // 分配连接内序号。
	session->out = session->message_out();                      // 创建输出消息，例如 RedisRequest/MySQLRequest。
	if (session->out)                                           // 输出消息存在。
		ret = this->send_message(entry);                        // 编码并发送请求。

	if (ret < 0)                                                // 发送失败。
	{
		entry->error = errno;                                   // 保存错误。
		mpoller_del(entry->sockfd, this->mpoller);              // 删除 fd 事件。
		entry->state = CONN_STATE_ERROR;                        // 标记连接错误。
		ret = 1;                                                // 返回 1，表示错误会异步回调处理。
	}

	pthread_mutex_unlock(&entry->mutex);                         // 解 entry 锁。
	return ret;                                                 // 0 全部写完，1 异步写/错误，-1 没有 idle。
}

int Communicator::request_new_conn(CommSession *session, CommTarget *target) // request_new_conn：新建连接发请求。
{
	struct CommConnEntry *entry;                                 // entry：新连接。
	struct poller_data data;                                     // data：connect 事件。
	int timeout;                                                 // timeout：连接超时。

	entry = Communicator::launch_conn(session, target);          // 创建非阻塞连接和 entry。
	if (entry)                                                  // 创建成功。
	{
		entry->mpoller = this->mpoller;                          // 记录 mpoller。
		session->conn = entry->conn;                             // session 记录连接对象。
		session->seq = entry->seq++;                             // 分配 session 序号。
		data.operation = PD_OP_CONNECT;                          // 注册 connect 事件。
		data.fd = entry->sockfd;                                 // fd：socket。
		data.ssl = NULL;                                        // 此处先不处理 SSL。
		data.context = entry;                                    // context：连接对象。
		timeout = session->target->connect_timeout;              // 使用 target 连接超时。
		if (mpoller_add(&data, timeout, this->mpoller) >= 0)     // 把非阻塞 connect 交给 poller 监听。
			return 0;                                           // 成功进入异步连接流程。

		__release_conn(entry);                                  // 注册失败，释放连接。
	}

	return -1;                                                  // 新建连接失败。
}

int Communicator::request(CommSession *session, CommTarget *target) // request(session,target)：客户端请求入口。
{
	int errno_bak;                                              // errno_bak：保存原 errno，成功路径恢复。

	if (session->passive)                                       // passive 表示服务端被动 session，不能走客户端 request。
	{
		errno = EINVAL;                                         // 参数错误。
		return -1;                                             // 返回失败。
	}

	errno_bak = errno;                                          // 保存当前 errno。
	session->target = target;                                   // session 绑定 target。
	session->out = NULL;                                        // 清空输出消息。
	session->in = NULL;                                         // 清空输入消息。
	if (this->request_idle_conn(session, target) < 0)            // 优先复用 idle 连接。
	{
		if (this->request_new_conn(session, target) < 0)         // 没有 idle 或复用失败，再新建连接。
		{
			session->conn = NULL;                                // 清空连接对象。
			session->seq = 0;                                    // 清空序号。
			return -1;                                          // 请求发起失败。
		}
	}

	errno = errno_bak;                                          // 成功发起时恢复 errno。
	return 0;                                                   // 请求进入异步流程。
}

int Communicator::nonblock_listen(CommService *service)         // nonblock_listen(service)：创建非阻塞监听 fd。
{
	int sockfd = service->create_listen_fd();                    // 让 service 创建监听 socket。
	int ret;                                                     // ret：listen 结果。

	if (sockfd >= 0)                                            // fd 创建成功。
	{
		if (__set_fd_nonblock(sockfd) >= 0)                      // 设置非阻塞。
		{
			/*
			  原始代码会检查 fd 是否已 bind，没有 bind 才 bind(service->bind_addr)。
			  这里保留主线理解：监听 fd 必须绑定地址，然后 listen。
			*/
			ret = listen(sockfd, SOMAXCONN);                     // 开始监听，backlog 使用系统最大值。
			if (ret >= 0 || errno == EOPNOTSUPP)                 // TCP listen 成功；UDP 不支持 listen 也可接受。
			{
				service->reliable = (ret >= 0);                  // reliable=true 表示 TCP 这类可靠连接。
				return sockfd;                                  // 返回监听 fd。
			}
		}

		close(sockfd);                                          // 失败则关闭 fd。
	}

	return -1;                                                  // 创建监听失败。
}

int Communicator::bind(CommService *service)                    // bind(service)：服务端启动入口。
{
	struct poller_data data;                                    // data：监听事件。
	int errno_bak = errno;                                      // 保存 errno。
	int sockfd;                                                 // sockfd：监听 fd。

	sockfd = this->nonblock_listen(service);                    // 创建非阻塞监听 fd。
	if (sockfd >= 0)                                            // 创建成功。
	{
		service->listen_fd = sockfd;                             // 保存到 service。
		service->ref = 1;                                       // service 引用计数初始为 1。
		data.fd = sockfd;                                      // poller 监听 fd。
		data.context = service;                                 // 回调上下文是 service。
		data.result = NULL;                                     // 初始 result 为空。
		if (service->reliable)                                  // TCP 这类可靠连接。
		{
			data.operation = PD_OP_LISTEN;                       // poller 操作是 listen/accept。
			data.accept = Communicator::accept;                  // accept 回调创建 target。
		}
		else                                                    // UDP 这类无连接服务。
		{
			data.operation = PD_OP_RECVFROM;                     // poller 操作是 recvfrom。
			data.recvfrom = Communicator::recvfrom;              // recvfrom 回调。
		}

		if (mpoller_add(&data, service->listen_timeout, this->mpoller) >= 0) // 把监听 fd 加入 mpoller。
		{
			errno = errno_bak;                                  // 成功时恢复 errno。
			return 0;                                           // bind 成功，服务开始接收连接。
		}

		close(sockfd);                                          // 注册失败，关闭 fd。
	}

	return -1;                                                  // bind 失败。
}

void Communicator::unbind(CommService *service)                 // unbind(service)：停止服务监听。
{
	int errno_bak = errno;                                      // 保存 errno。

	if (mpoller_del(service->listen_fd, this->mpoller) < 0)      // 从 mpoller 删除监听 fd。
	{
		/* Error occurred on listen_fd or Communicator::deinit() called. */
		this->shutdown_service(service);                        // 如果删除失败，直接关闭 service。
		errno = errno_bak;                                      // 恢复 errno。
	}
}

void *Communicator::accept(const struct sockaddr *addr, socklen_t addrlen, // addr/addrlen：客户端地址。
						   int sockfd, void *context)                      // sockfd：accept 得到的新连接 fd；context：service。
{
	CommService *service = (CommService *)context;               // service：当前服务。
	CommServiceTarget *target = new CommServiceTarget;           // 每个服务端连接也包装成一个 target。

	if (target)                                                 // 分配成功。
	{
		if (target->init(addr, addrlen, 0, service->response_timeout) >= 0) // 初始化目标地址和响应超时。
		{
			service->incref();                                  // service 引用计数加一，连接存在时服务不能完全释放。
			target->service = service;                          // target 指回 service。
			target->sockfd = sockfd;                            // 保存连接 fd。
			target->ref = 1;                                    // target 引用计数初始为 1。
			return target;                                      // 返回给 poller，后续创建连接 entry。
		}

		delete target;                                          // 初始化失败，释放 target。
	}

	close(sockfd);                                              // 失败时关闭连接 fd。
	return NULL;                                                // accept 处理失败。
}

poller_message_t *Communicator::create_request(void *context)   // create_request(context)：服务端收到请求数据时创建请求消息。
{
	struct CommConnEntry *entry = (struct CommConnEntry *)context; // entry：当前服务端连接。
	CommService *service = entry->service;                       // service：所属服务。
	CommTarget *target = entry->target;                          // target：服务端连接 target。
	CommSession *session;                                       // session：新创建的服务端任务会话。
	CommMessageIn *in;                                          // in：输入消息，例如 HttpRequest。
	int timeout;                                                // timeout：接收请求超时。

	session = service->new_session(entry->seq, entry->conn);     // 由 WFServer 创建协议相关 session，例如 WFHttpTask。
	if (!session)                                               // 创建失败。
		return NULL;                                            // 返回 NULL。

	session->passive = 1;                                       // passive=1 表示服务端被动 session。
	entry->session = session;                                   // entry 绑定 session。
	session->target = target;                                   // session 绑定 target。
	session->conn = entry->conn;                                // session 绑定连接对象。
	session->seq = entry->seq++;                                // 分配连接内序号。
	session->out = NULL;                                        // 服务端先没有输出，处理完请求后才有 response。
	session->in = NULL;                                         // 输入稍后创建。

	timeout = Communicator::first_timeout_recv(session);         // 计算接收请求超时。
	mpoller_set_timeout(entry->sockfd, timeout, entry->mpoller); // 设置 fd 超时。
	entry->state = CONN_STATE_RECEIVING;                         // 状态改为接收中。

	((CommServiceTarget *)target)->incref();                     // target 引用加一，session 持有连接期间不能释放。

	in = session->message_in();                                 // 创建输入消息，例如 HttpRequest。
	if (in)                                                     // 创建成功。
	{
		in->poller_message_t::append = Communicator::append_message; // 设置 poller 收到字节后的 append 函数。
		in->entry = entry;                                      // 输入消息记录所属连接。
		session->in = in;                                       // session 保存输入消息。
	}

	return in;                                                  // 返回给 poller，后续 poller 将网络字节 append 到它。
}

poller_message_t *Communicator::create_reply(void *context)     // create_reply(context)：客户端收到响应数据时创建响应消息。
{
	struct CommConnEntry *entry = (struct CommConnEntry *)context; // entry：客户端连接。
	CommSession *session;                                      // session：当前请求会话。
	CommMessageIn *in;                                         // in：响应消息，例如 RedisResponse/MySQLResponse。

	if (entry->state != CONN_STATE_RECEIVING)                   // 只有等待响应状态才允许创建 reply。
	{
		errno = EBADMSG;                                        // 状态不对，认为消息错误。
		return NULL;                                           // 返回失败。
	}

	session = entry->session;                                   // 取当前 session。
	in = session->message_in();                                // 创建输入消息对象。
	if (in)                                                     // 创建成功。
	{
		in->poller_message_t::append = Communicator::append_message; // 设置 append 函数。
		in->entry = entry;                                      // 记录连接。
		session->in = in;                                       // 保存到 session。
	}

	return in;                                                  // 返回给 poller。
}

int Communicator::partial_written(size_t n, void *context)      // partial_written：异步写过程中部分写入后的回调。
{
	struct CommConnEntry *entry = (struct CommConnEntry *)context; // entry：当前连接。
	CommSession *session = entry->session;                       // session：当前会话。
	int timeout;                                                // timeout：剩余超时。

	timeout = Communicator::next_timeout(session);               // 计算剩余发送超时。
	mpoller_set_timeout(entry->sockfd, timeout, entry->mpoller); // 更新 fd 超时。
	return 0;                                                   // 返回成功。
}

/*
  下面这几个 handle_* 函数是事件完成后从 poller_result 回到 session->handle() 的关键路径。
  你面试讲原理时可以这样说：

  poller 线程只负责监听 fd 事件，事件完成后把 poller_result 放进 msgqueue；
  handler 线程从 msgqueue 取出结果，根据是读、写、连接、监听等事件调用对应 handle 函数；
  handle 函数最终调用 CommSession::handle(state, error)，再由 CommRequest::handle 调用 subtask_done()。
*/

void Communicator::handle_incoming_reply(struct poller_result *res) // handle_incoming_reply：客户端响应读完成。
{
	struct CommConnEntry *entry = (struct CommConnEntry *)res->data.context; // entry：事件所属连接。
	CommTarget *target = entry->target;                         // target：连接目标。
	CommSession *session = NULL;                                // session：最终要回调的会话。
	int state;                                                  // state：传给 session->handle 的状态。

	switch (res->state)                                         // 根据 poller 结果状态判断。
	{
	case PR_ST_SUCCESS:                                         // 成功读到完整响应。
		session = entry->session;                               // 取当前 session。
		state = CS_STATE_SUCCESS;                               // 通信状态成功。
		pthread_mutex_lock(&target->mutex);                      // 锁住 target。
		if (entry->state == CONN_STATE_SUCCESS)                 // 如果 append_message 已把连接标记为成功。
		{
			__sync_add_and_fetch(&entry->ref, 1);                // 增加引用，准备放入 idle_list。
			if (session->timeout != 0)                           // timeout != 0 表示可以 keep-alive。
			{
				entry->state = CONN_STATE_IDLE;                  // 连接变为空闲。
				list_add(&entry->list, &target->idle_list);      // 加入 target 空闲连接链表，后续请求可复用。
			}
			else
				entry->state = CONN_STATE_CLOSING;               // 不保持连接，准备关闭。
		}

		pthread_mutex_unlock(&target->mutex);                    // 解锁。
		break;

	case PR_ST_ERROR:                                           // 读响应出错。
	case PR_ST_FINISHED:                                        // 对端关闭连接。
		state = CS_STATE_ERROR;                                 // 对 session 来说是通信错误。
		session = entry->session;                               // 回调当前 session。
		break;

	case PR_ST_DELETED:                                         // fd 被删除。
	case PR_ST_STOPPED:                                         // poller 停止。
	default:
		state = CS_STATE_STOPPED;                               // 标记停止。
		session = entry->session;                               // 回调当前 session。
		break;
	}

	if (session)                                                // 如果有 session 需要通知。
	{
		target->release();                                      // 释放 CommSchedTarget 占用，唤醒等待连接名额的请求。
		session->handle(state, res->error);                      // 回到 CommSession::handle，最终推进 Workflow 任务。
	}

	if (__sync_sub_and_fetch(&entry->ref, 1) == 0)               // 减少连接引用，如果归零。
		__release_conn(entry);                                  // 释放连接。
}

void Communicator::handle_request_result(struct poller_result *res) // handle_request_result：客户端请求写完成。
{
	struct CommConnEntry *entry = (struct CommConnEntry *)res->data.context; // entry：当前连接。
	CommSession *session = entry->session;                       // session：当前请求。
	int timeout;                                                // timeout：接收响应超时。
	int state;                                                  // state：错误时回调状态。

	switch (res->state)                                         // 根据写事件结果处理。
	{
	case PR_ST_FINISHED:                                        // 请求已经完整写出。
		entry->state = CONN_STATE_RECEIVING;                     // 进入等待响应状态。
		res->data.operation = PD_OP_READ;                        // 下一步监听读事件。
		res->data.create_message = Communicator::create_reply;   // 收到响应字节时创建 reply message。
		res->data.message = NULL;                                // message 先为空，由 create_reply 创建。
		timeout = session->first_timeout();                      // 先看 session 自定义 first timeout。
		if (timeout == 0)                                       // 如果没有特殊 first timeout。
			timeout = Communicator::first_timeout_recv(session);  // 使用接收响应超时。
		else                                                    // 有特殊超时。
		{
			session->timeout = -1;                               // 重置 session 超时。
			session->begin_time.tv_sec = -1;                     // begin_time 置无效。
			session->begin_time.tv_nsec = 0;
		}

		if (mpoller_add(&res->data, timeout, this->mpoller) >= 0) // 把 fd 读事件加入 poller，等待响应。
		{
			if (this->stop_flag)                                // 如果正在停止。
				mpoller_del(res->data.fd, this->mpoller);        // 删除事件。
			break;                                              // 正常进入异步接收。
		}

		res->error = errno;                                     // 注册读事件失败，记录错误。
		/* fall through */                                      // 进入错误处理。

	case PR_ST_ERROR:                                           // 写请求失败。
	case PR_ST_DELETED:
	case PR_ST_STOPPED:
		state = (res->state == PR_ST_ERROR ? CS_STATE_ERROR : CS_STATE_STOPPED); // 转换状态。
		entry->target->release();                               // 释放连接名额。
		session->handle(state, res->error);                      // 回调 session，任务失败完成。
		if (__sync_sub_and_fetch(&entry->ref, 1) == 0)           // 引用归零则释放连接。
			__release_conn(entry);
		break;
	}
}

void Communicator::handle_poller_result(struct poller_result *res) // handle_poller_result：handler 线程处理 poller 结果的统一入口。
{
	switch (res->data.operation)                                // 根据 poller 操作类型分发。
	{
	case PD_OP_CONNECT:                                         // 非阻塞 connect 完成。
		/*
		  原始代码会处理 SSL 握手、创建 message_out、发送请求。
		  主线理解：connect 成功后调用 send_message；
		  send_message 成功写完后转入读响应，未写完则注册写事件。
		*/
		break;

	case PD_OP_WRITE:                                           // 写事件完成。
		this->handle_request_result(res);                       // 客户端请求写完，转入读响应。
		break;

	case PD_OP_READ:                                            // 读事件完成。
		this->handle_incoming_reply(res);                       // 客户端收到响应，回调 session。
		break;

	case PD_OP_LISTEN:                                          // 服务端监听事件，有新连接。
		/*
		  原始代码会创建 CommConnEntry，并注册读事件；
		  当请求数据到来时 create_request 创建 WFServerTask/WFHttpTask。
		*/
		break;

	default:                                                    // 其他事件，如 timer、AIO、notify。
		break;                                                  // 学习版省略。
	}
}

void Communicator::handler_thread_routine(void *context)        // handler_thread_routine：handler 线程主循环。
{
	Communicator *comm = (Communicator *)context;                // context 是 Communicator。
	struct poller_result res;                                    // res：从 msgqueue 取出的事件结果。

	while (msgqueue_get(&res, comm->msgqueue) >= 0)              // 持续从队列取 poller_result。
	{
		if (res.data.operation == -1)                            // operation == -1 是减少 handler 线程的退出信号。
			break;                                              // 当前 handler 退出。

		comm->handle_poller_result(&res);                        // 处理事件结果。
	}
}

int Communicator::is_handler_thread() const                     // is_handler_thread：判断当前线程是否属于 handler 线程池。
{
	return thrdpool_in_pool(this->thrdpool);                     // 调用 thrdpool 判断。
}

int Communicator::increase_handler_thread()                     // increase_handler_thread：动态增加 handler 线程。
{
	void *buf = malloc(4 * sizeof (void *));                     // 原始线程池内部调度需要的小缓冲。

	if (buf)                                                    // 分配成功。
	{
		if (thrdpool_increase(this->thrdpool) >= 0)              // 线程池增加一个线程。
		{
			struct thrdpool_task task = {                        // 新线程要运行的 handler 常驻任务。
				.routine	=	Communicator::handler_thread_routine,
				.context	=	this
			};
			__thrdpool_schedule(&task, buf, this->thrdpool);     // 把 handler routine 投递给新增线程。
			return 0;                                           // 增加成功。
		}

		free(buf);                                              // 增加失败，释放缓冲。
	}

	return -1;                                                  // 增加失败。
}

int Communicator::decrease_handler_thread()                     // decrease_handler_thread：动态减少 handler 线程。
{
	struct poller_result *res;                                  // res：伪造的退出事件。
	size_t size;                                                // size：分配大小。

	size = sizeof (struct poller_result) + sizeof (void *);      // 原始队列元素可能带额外指针空间。
	res = (struct poller_result *)malloc(size);                  // 分配退出事件。
	if (res)                                                    // 分配成功。
	{
		res->data.operation = -1;                               // operation=-1 表示 handler 线程退出。
		msgqueue_put_head(res, this->msgqueue);                 // 放到队头，让一个 handler 尽快拿到并退出。
		return 0;                                               // 请求减少成功。
	}

	return -1;                                                  // 分配失败。
}

/*
  面试总结口径：

  1. 上层 Redis/MySQL/HTTP client task 最终调用 CommScheduler::request。
  2. CommScheduler 先 acquire target，控制 max_connections。
  3. Communicator::request 优先 request_idle_conn 复用连接，没有 idle 才 request_new_conn。
  4. 新连接用 nonblock_connect + mpoller_add(PD_OP_CONNECT)，不会阻塞业务线程。
  5. 请求发送用协议对象 encode 成 iovec，再 writev；没写完就注册 PD_OP_WRITE。
  6. 写完后注册 PD_OP_READ 等响应。
  7. poller 线程拿到 fd 事件后不直接执行业务回调，而是把 poller_result 放入 msgqueue。
  8. handler 线程从 msgqueue 取事件，调用 session->handle。
  9. session->handle 最终触发 CommRequest::handle，再 subtask_done，Workflow 串行流继续往下走。
*/

