/*
  注释版源码文件：Communicator.h

  原始文件位置：
  workflow-master/src/kernel/Communicator.h

  本文件用途：
  Communicator 是 Workflow 最底层的异步通信引擎。

  对高并发面试最重要：
  1. 它不是一请求一线程。
  2. 它通过 poller/mpoller 监听 socket 事件。
  3. 它通过 handler 线程处理事件结果和回调。
  4. 它统一接入网络请求、服务监听、定时器、文件 IO。

  一句话：
  Communicator 是“真正干活”的底层引擎。
*/

#ifndef _COMMUNICATOR_H_                                      // 头文件保护宏。
#define _COMMUNICATOR_H_                                      // 定义头文件保护宏。

#include <sys/types.h>                                        // 系统类型。
#include <sys/socket.h>                                       // socket/sockaddr。
#include <sys/uio.h>                                          // iovec，用于分散/聚集 IO。
#include <time.h>                                             // timespec。
#include <stddef.h>                                           // size_t。
#include <pthread.h>                                          // pthread mutex 等。
#include <openssl/ssl.h>                                      // SSL_CTX/SSL。
#include "list.h"                                             // 内核风格链表。
#include "poller.h"                                           // poller 事件模型。

class CommConnection                                          // CommConnection：通信连接的基础对象。
{
public:
	virtual ~CommConnection() { }                              // 虚析构函数；子类 WFConnection 可以继承。
};

class CommTarget                                              // CommTarget：一个远端目标，比如 Redis/MySQL/HTTP 服务器地址。
{
public:
	int init(const struct sockaddr *addr, socklen_t addrlen,    // addr/addrlen：目标地址。
			 int connect_timeout,                              // connect_timeout：建立连接超时。
			 int response_timeout);                            // response_timeout：单次响应超时。
	void deinit();                                             // deinit()：释放目标资源。

public:
	void get_addr(const struct sockaddr **addr, socklen_t *addrlen) const // get_addr()：获取目标地址。
	{
		*addr = this->addr;                                    // 输出目标地址指针。
		*addrlen = this->addrlen;                              // 输出目标地址长度。
	}

	int has_idle_conn() const { return !list_empty(&this->idle_list); } // has_idle_conn()：是否有空闲连接可复用。

protected:
	void set_ssl(SSL_CTX *ssl_ctx, int ssl_connect_timeout)     // set_ssl()：设置 SSL 上下文和 SSL 连接超时。
	{
		this->ssl_ctx = ssl_ctx;                               // 保存 SSL_CTX。
		this->ssl_connect_timeout = ssl_connect_timeout;       // 保存 SSL 握手超时。
	}

	SSL_CTX *get_ssl_ctx() const { return this->ssl_ctx; }      // get_ssl_ctx()：获取 SSL 上下文。

private:
	virtual int create_connect_fd()                            // create_connect_fd()：创建主动连接 socket。
	{
		return socket(this->addr->sa_family, SOCK_STREAM, 0);   // 默认创建 TCP socket。
	}

	virtual CommConnection *new_connection(int connect_fd)      // new_connection(connect_fd)：连接建立后创建连接对象。
	{
		return new CommConnection;                             // 默认创建基础连接对象。
	}

	virtual int init_ssl(SSL *ssl) { return 0; }                // init_ssl(ssl)：子类可覆盖，初始化 SSL。

public:
	virtual void release() { }                                 // release()：目标使用结束后释放；CommSchedTarget 会覆盖。

private:
	struct sockaddr *addr;                                     // addr：目标地址。
	socklen_t addrlen;                                         // addrlen：目标地址长度。
	int connect_timeout;                                       // connect_timeout：连接超时。
	int response_timeout;                                      // response_timeout：响应超时。
	int ssl_connect_timeout;                                   // ssl_connect_timeout：SSL 握手超时。
	SSL_CTX *ssl_ctx;                                          // ssl_ctx：SSL 上下文。

private:
	struct list_head idle_list;                                // idle_list：空闲连接链表，用于 keep-alive 复用。
	pthread_mutex_t mutex;                                     // mutex：保护 idle_list 等目标内部状态。

public:
	virtual ~CommTarget() { }                                  // 虚析构函数。
	friend class CommServiceTarget;                            // 服务端目标类可访问内部字段。
	friend class Communicator;                                 // Communicator 需要访问目标内部连接池。
};

class CommMessageOut                                          // CommMessageOut：可发送消息接口。
{
private:
	virtual int encode(struct iovec vectors[], int max) = 0;    // encode(vectors,max)：把消息编码成 iovec 数组。

public:
	virtual ~CommMessageOut() { }                              // 虚析构函数。
	friend class Communicator;                                 // Communicator 调用 private encode()。
};

class CommMessageIn : private poller_message_t                 // CommMessageIn：可接收消息接口，同时适配 poller_message_t。
{
private:
	virtual int append(const void *buf, size_t *size) = 0;      // append(buf,size)：把收到的字节追加到协议解析器。

protected:
	virtual int feedback(const void *buf, size_t size);         // feedback(buf,size)：接收过程中发送小包反馈，比如 100-continue。

	virtual void renew();                                      // renew()：重置接收开始时间，用于刷新超时。

	virtual CommMessageIn *inner() { return this; }             // inner()：返回最内层消息对象，包装协议可覆盖。

private:
	struct CommConnEntry *entry;                               // entry：当前消息所属连接条目。

public:
	virtual ~CommMessageIn() { }                               // 虚析构函数。
	friend class Communicator;                                 // Communicator 需要访问 append/entry。
};

#define CS_STATE_SUCCESS	0                                  // 通信成功。
#define CS_STATE_ERROR		1                                  // 通信错误。
#define CS_STATE_STOPPED	2                                  // 通信被停止。
#define CS_STATE_TOREPLY	3                                  // 服务端专用：收到请求，准备回复。

class CommSession                                             // CommSession：一次通信会话。
{
private:
	virtual CommMessageOut *message_out() = 0;                 // message_out()：返回要发送的消息对象。
	virtual CommMessageIn *message_in() = 0;                   // message_in()：返回要接收解析的消息对象。
	virtual int send_timeout() { return -1; }                  // send_timeout()：发送完整消息超时。
	virtual int receive_timeout() { return -1; }               // receive_timeout()：接收完整消息超时。
	virtual int keep_alive_timeout() { return 0; }             // keep_alive_timeout()：连接保活超时。
	virtual int first_timeout() { return 0; }                  // first_timeout()：首包等待超时。
	virtual void handle(int state, int error) = 0;             // handle(state,error)：通信完成后回调给任务。

protected:
	CommTarget *get_target() const { return this->target; }    // get_target()：获取当前目标。
	CommConnection *get_connection() const { return this->conn; } // get_connection()：获取当前连接。
	CommMessageOut *get_message_out() const { return this->out; } // get_message_out()：获取当前输出消息。
	CommMessageIn *get_message_in() const { return this->in; } // get_message_in()：获取当前输入消息。
	long long get_seq() const { return this->seq; }            // get_seq()：获取会话序号。

private:
	CommTarget *target;                                        // target：当前会话目标。
	CommConnection *conn;                                      // conn：当前会话连接。
	CommMessageOut *out;                                       // out：当前发送消息。
	CommMessageIn *in;                                         // in：当前接收消息。
	long long seq;                                             // seq：会话序号。

private:
	struct timespec begin_time;                                // begin_time：当前阶段开始时间，用于超时判断。
	int timeout;                                               // timeout：当前阶段超时时间。
	int passive;                                               // passive：是否为被动服务端会话。

public:
	CommSession() { this->passive = 0; }                       // 构造函数，默认主动客户端会话。
	virtual ~CommSession();                                    // 析构函数。
	friend class CommMessageIn;                                // CommMessageIn 可访问 entry/session。
	friend class Communicator;                                 // Communicator 驱动整个会话。
};

class CommService                                             // CommService：服务端监听抽象。
{
public:
	int init(const struct sockaddr *bind_addr, socklen_t addrlen, // bind_addr/addrlen：监听地址。
			 int listen_timeout,                               // listen_timeout：accept/listen 相关超时。
			 int response_timeout);                            // response_timeout：对端响应超时。
	void deinit();                                             // deinit()：释放服务资源。

	int drain(int max);                                        // drain(max)：主动处理/释放部分 keep-alive 连接。

public:
	void get_addr(const struct sockaddr **addr, socklen_t *addrlen) const // get_addr()：获取监听地址。
	{
		*addr = this->bind_addr;                               // 输出监听地址。
		*addrlen = this->addrlen;                              // 输出地址长度。
	}

protected:
	void set_ssl(SSL_CTX *ssl_ctx, int ssl_accept_timeout)      // set_ssl()：设置服务端 SSL。
	{
		this->ssl_ctx = ssl_ctx;                               // 保存 SSL 上下文。
		this->ssl_accept_timeout = ssl_accept_timeout;         // 保存 SSL accept 握手超时。
	}

	SSL_CTX *get_ssl_ctx() const { return this->ssl_ctx; }      // 获取 SSL_CTX。

private:
	virtual CommSession *new_session(long long seq, CommConnection *conn) = 0; // new_session()：新连接/请求到来时创建会话。
	virtual void handle_stop(int error) { }                    // handle_stop(error)：服务停止时通知。
	virtual void handle_unbound() = 0;                         // handle_unbound()：服务解绑完成时通知。

private:
	virtual int create_listen_fd()                             // create_listen_fd()：创建监听 socket。
	{
		return socket(this->bind_addr->sa_family, SOCK_STREAM, 0); // 默认 TCP socket。
	}

	virtual CommConnection *new_connection(int accept_fd)       // new_connection(accept_fd)：accept 后创建连接对象。
	{
		return new CommConnection;                             // 默认基础连接。
	}

	virtual int init_ssl(SSL *ssl) { return 0; }                // init_ssl(ssl)：服务端 SSL 初始化钩子。

private:
	struct sockaddr *bind_addr;                                // bind_addr：监听地址。
	socklen_t addrlen;                                         // addrlen：监听地址长度。
	int listen_timeout;                                        // listen_timeout：监听/接收超时。
	int response_timeout;                                      // response_timeout：对端响应超时。
	int ssl_accept_timeout;                                    // ssl_accept_timeout：SSL accept 超时。
	SSL_CTX *ssl_ctx;                                          // ssl_ctx：SSL 上下文。

private:
	void incref();                                             // incref()：服务引用计数加 1。
	void decref();                                             // decref()：服务引用计数减 1。

private:
	int reliable;                                              // reliable：是否可靠服务连接。
	int listen_fd;                                             // listen_fd：监听 socket。
	int ref;                                                   // ref：引用计数。

private:
	struct list_head keep_alive_list;                          // keep_alive_list：服务端 keep-alive 连接列表。
	pthread_mutex_t mutex;                                     // mutex：保护服务内部状态。

public:
	virtual ~CommService() { }                                 // 虚析构函数。
	friend class CommServiceTarget;                            // 服务目标类可访问内部。
	friend class Communicator;                                 // Communicator 负责 bind/unbind/accept。
};

#define SS_STATE_COMPLETE	0                                  // SleepSession 完成。
#define SS_STATE_ERROR		1                                  // SleepSession 错误。
#define SS_STATE_DISRUPTED	2                                  // SleepSession 被打断。

class SleepSession                                            // SleepSession：定时器会话。
{
private:
	virtual int duration(struct timespec *value) = 0;           // duration(value)：输出定时时长。
	virtual void handle(int state, int error) = 0;              // handle(state,error)：定时完成/失败回调。

private:
	void *timer;                                               // timer：底层 timer 句柄。
	int index;                                                 // index：所属 poller/timer 索引。

public:
	virtual ~SleepSession() { }                                // 虚析构函数。
	friend class Communicator;                                 // Communicator 调用 duration/handle。
};

#ifdef __linux__                                              // Linux 平台。
# include "IOService_linux.h"                                 // Linux 使用原生 AIO/IOService 实现。
#else                                                         // 非 Linux。
# include "IOService_thread.h"                                // 其他平台使用线程模拟文件 IO。
#endif

class CommEventHandler                                        // CommEventHandler：自定义事件处理器接口。
{
private:
	virtual void schedule(void (*routine)(void *), void *context) = 0; // schedule()：调度一个回调函数。
	virtual void wait() = 0;                                  // wait()：等待事件处理完成。

public:
	virtual ~CommEventHandler() { }                            // 虚析构函数。
	friend class Communicator;                                 // Communicator 使用它替代默认 handler 线程池。
};

class Communicator                                            // Communicator：底层通信引擎。
{
public:
	int init(size_t poller_threads, size_t handler_threads);    // init()：创建 poller 线程和 handler 线程。
	void deinit();                                             // deinit()：销毁通信引擎。

	int request(CommSession *session, CommTarget *target);      // request()：客户端发起请求。
	int reply(CommSession *session);                           // reply()：服务端发送响应。

	int push(const void *buf, size_t size, CommSession *session); // push()：向连接直接推送数据。

	int shutdown(CommSession *session);                        // shutdown()：关闭会话连接。

	int bind(CommService *service);                            // bind()：绑定服务监听。
	void unbind(CommService *service);                         // unbind()：取消服务监听。

	int sleep(SleepSession *session);                          // sleep()：注册定时器。
	int unsleep(SleepSession *session);                        // unsleep()：取消定时器。

	int io_bind(IOService *service);                           // io_bind()：绑定文件 IO 服务。
	void io_unbind(IOService *service);                        // io_unbind()：解绑文件 IO 服务。

public:
	int is_handler_thread() const;                             // is_handler_thread()：判断当前是否 handler 线程。

	int increase_handler_thread();                             // increase_handler_thread()：增加 handler 线程。
	int decrease_handler_thread();                             // decrease_handler_thread()：减少 handler 线程。

public:
	void customize_event_handler(CommEventHandler *handler);    // customize_event_handler()：设置自定义事件处理器。

private:
	struct __mpoller *mpoller;                                 // mpoller：多 poller 对象，负责监听 fd/timer/aio 事件。
	struct __msgqueue *msgqueue;                               // msgqueue：poller 到 handler 的消息队列。
	struct __thrdpool *thrdpool;                               // thrdpool：handler 线程池。
	int stop_flag;                                             // stop_flag：停止标记。

private:
	CommEventHandler *event_handler;                           // event_handler：自定义事件处理器，默认为空。

private:
	int create_poller(size_t poller_threads);                  // create_poller()：创建 poller 线程。

	int create_handler_threads(size_t handler_threads);         // create_handler_threads()：创建 handler 线程。

	void shutdown_service(CommService *service);                // shutdown_service()：关闭服务。

	void shutdown_io_service(IOService *service);               // shutdown_io_service()：关闭 IOService。

	int send_message_sync(struct iovec vectors[], int cnt,
						  struct CommConnEntry *entry);        // 同步发送消息。
	int send_message_async(struct iovec vectors[], int cnt,
						   struct CommConnEntry *entry);       // 异步发送消息。

	int send_message(struct CommConnEntry *entry);              // send_message()：根据连接状态发送消息。

	int request_new_conn(CommSession *session, CommTarget *target); // 使用新连接发起请求。
	int request_idle_conn(CommSession *session, CommTarget *target); // 使用空闲连接发起请求。

	int reply_message_unreliable(struct CommConnEntry *entry);  // 非可靠连接回复消息。

	int reply_reliable(CommSession *session, CommTarget *target); // 可靠连接回复。
	int reply_unreliable(CommSession *session, CommTarget *target); // 非可靠连接回复。

	void handle_poller_result(struct poller_result *res);       // 统一处理 poller 返回事件。

	void handle_incoming_request(struct poller_result *res);    // 处理服务端收到请求。
	void handle_incoming_reply(struct poller_result *res);      // 处理客户端收到回复。

	void handle_request_result(struct poller_result *res);      // 处理请求发送结果。
	void handle_reply_result(struct poller_result *res);        // 处理回复发送结果。

	void handle_write_result(struct poller_result *res);        // 处理写事件结果。
	void handle_read_result(struct poller_result *res);         // 处理读事件结果。

	void handle_connect_result(struct poller_result *res);      // 处理 connect 完成事件。
	void handle_listen_result(struct poller_result *res);       // 处理 listen/accept 事件。

	void handle_recvfrom_result(struct poller_result *res);     // 处理 recvfrom 事件。

	void handle_ssl_accept_result(struct poller_result *res);   // 处理 SSL accept 结果。

	void handle_sleep_result(struct poller_result *res);        // 处理定时器结果。

	void handle_aio_result(struct poller_result *res);          // 处理文件 AIO 结果。

	static void handler_thread_routine(void *context);          // handler 线程主函数。

	static int nonblock_connect(CommTarget *target);            // 创建非阻塞连接。
	static int nonblock_listen(CommService *service);           // 创建非阻塞监听。

	static struct CommConnEntry *launch_conn(CommSession *session,
											 CommTarget *target); // 创建客户端连接条目。
	static struct CommConnEntry *accept_conn(class CommServiceTarget *target,
											 CommService *service); // 创建服务端 accept 连接条目。

	static int first_timeout(CommSession *session);             // 计算首次超时。
	static int next_timeout(CommSession *session);              // 计算后续阶段超时。

	static int first_timeout_send(CommSession *session);        // 计算发送首阶段超时。
	static int first_timeout_recv(CommSession *session);        // 计算接收首阶段超时。

	static int append_message(const void *buf, size_t *size,
							  poller_message_t *msg);          // poller 调用：把收到数据追加到消息对象。

	static poller_message_t *create_request(void *context);     // 创建请求消息解析对象。
	static poller_message_t *create_reply(void *context);       // 创建响应消息解析对象。

	static int recv_request(const void *buf, size_t size,
							struct CommConnEntry *entry);      // 接收请求数据。

	static int partial_written(size_t n, void *context);        // 部分写入回调。

	static void *accept(const struct sockaddr *addr, socklen_t addrlen,
						int sockfd, void *context);            // accept 回调，创建新连接上下文。

	static void *recvfrom(const struct sockaddr *addr, socklen_t addrlen,
						  const void *buf, size_t size, void *context); // recvfrom 回调。

	static void callback(struct poller_result *res, void *context); // poller 事件回调入口。

private:
	static void event_handler_routine(void *context);           // 自定义 event handler 使用的例程。

	static void callback_custom(struct poller_result *res, void *context); // 自定义 handler 的 poller 回调。

public:
	virtual ~Communicator() { }                                // 虚析构函数。
};

#endif                                                        // 结束头文件保护宏。
