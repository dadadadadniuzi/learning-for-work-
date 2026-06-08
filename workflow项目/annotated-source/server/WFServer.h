/*
  注释版源码文件：WFServer.h

  原始文件位置：
  workflow-master/src/server/WFServer.h

  本文件用途：
  定义 Workflow 的通用服务端模板。

  对随机图库项目来说：
  1. 你写的 `WFHttpServer server([](WFHttpTask *task) { ... })` 最终会用到这里的 WFServer。
  2. `WFServerBase` 负责监听、启动、停止、连接数、SSL、服务生命周期。
  3. `WFServer<REQ, RESP>` 负责把“协议请求/响应类型”和“用户 process 回调”绑定起来。
  4. 当有新连接/新请求到来时，会通过 `new_session()` 创建一个服务端网络任务。
*/

#ifndef _WFSERVER_H_                                          // 头文件保护宏：防止重复 include。
#define _WFSERVER_H_                                          // 定义头文件保护宏。

#include <sys/types.h>                                        // 引入系统类型，如 socklen_t 等。
#include <sys/socket.h>                                       // 引入 socket 地址结构和 getsockname 等接口。
#include <errno.h>                                            // 引入 errno；出错时设置 ENOTCONN 等错误码。
#include <functional>                                         // 引入 std::function，用来保存用户 process 回调。
#include <atomic>                                             // 引入 std::atomic，用于原子连接计数。
#include <mutex>                                              // 引入 std::mutex，用于服务停止等待的互斥保护。
#include <condition_variable>                                 // 引入 condition_variable，用于 wait_finish 等待服务完全解绑。
#include <openssl/ssl.h>                                      // 引入 SSL_CTX/SSL，HTTPS 服务需要。
#include "EndpointParams.h"                                   // 引入 TransportType、EndpointParams 等网络端点参数。
#include "WFTaskFactory.h"                                    // 引入 WFNetworkTaskFactory，用于创建服务端任务。

struct WFServerParams                                         // WFServerParams：服务端运行参数。
{
	enum TransportType transport_type;                         // transport_type：传输类型，通常是 TT_TCP。
	size_t max_connections;                                    // max_connections：最大连接数，防止连接无限增长。
	int peer_response_timeout;                                 // peer_response_timeout：单次读/写对端响应超时，单位毫秒。
	int receive_timeout;                                       // receive_timeout：接收完整请求的总超时，-1 表示无限。
	int keep_alive_timeout;                                    // keep_alive_timeout：连接保持时间，单位毫秒。
	size_t request_size_limit;                                 // request_size_limit：请求体大小限制，防止超大请求占用内存。
	int ssl_accept_timeout;                                    // ssl_accept_timeout：SSL 握手超时；非 SSL 服务忽略。
};

static constexpr struct WFServerParams SERVER_PARAMS_DEFAULT = // SERVER_PARAMS_DEFAULT：通用服务端默认配置。
{
	.transport_type			=	TT_TCP,                         // 默认使用 TCP。
	.max_connections		=	2000,                           // 默认最多 2000 个连接。
	.peer_response_timeout	=	10 * 1000,                     // 默认单次读写超时 10 秒。
	.receive_timeout		=	-1,                             // 默认接收完整请求不设总超时。
	.keep_alive_timeout		=	60 * 1000,                     // 默认 keep-alive 60 秒。
	.request_size_limit		=	(size_t)-1,                    // 默认请求大小不限制。
	.ssl_accept_timeout		=	10 * 1000,                     // 默认 SSL 握手超时 10 秒。
};

class WFServerBase : protected CommService                     // WFServerBase：所有服务端的基础类，底层继承 CommService。
{
public:
	WFServerBase(const struct WFServerParams *params) :        // 构造函数参数 params：服务端配置。
		conn_count(0)                                          // conn_count：当前连接数，初始化为 0。
	{
		this->params = *params;                                // 拷贝用户传入的服务端参数，后续启动和创建任务时使用。
		this->unbind_finish = false;                           // unbind_finish：服务是否完成解绑，初始为 false。
		this->listen_fd = -1;                                  // listen_fd：监听 socket，-1 表示尚未启动监听。
	}

public:
	int start(unsigned short port)                             // start(port)：用 IPv4 在指定端口启动 TCP 服务。
	{
		return start(AF_INET, NULL, port, NULL, NULL);          // 调用更通用的 start，host/cert/key 都为空。
	}

	int start(int family, unsigned short port)                 // start(family, port)：用指定地址族启动服务。
	{
		return start(family, NULL, port, NULL, NULL);           // family 可以是 AF_INET 或 AF_INET6。
	}

	int start(const char *host, unsigned short port)            // start(host, port)：绑定指定主机和端口启动服务。
	{
		return start(AF_INET, host, port, NULL, NULL);          // 默认 IPv4，不使用 SSL。
	}

	int start(int family, const char *host, unsigned short port) // start(family, host, port)：指定地址族、主机、端口。
	{
		return start(family, host, port, NULL, NULL);           // 不使用 SSL。
	}

	int start(const struct sockaddr *bind_addr, socklen_t addrlen) // start(bind_addr, addrlen)：直接用 socket 地址启动。
	{
		return start(bind_addr, addrlen, NULL, NULL);           // 不使用 SSL。
	}

	int start(unsigned short port, const char *cert_file, const char *key_file) // start(port, cert, key)：启动 SSL 服务。
	{
		return start(AF_INET, NULL, port, cert_file, key_file); // IPv4 + SSL 证书文件。
	}

	int start(int family, unsigned short port,
			  const char *cert_file, const char *key_file)      // 指定地址族和 SSL 证书启动。
	{
		return start(family, NULL, port, cert_file, key_file);  // host 为空表示绑定默认地址。
	}

	int start(const char *host, unsigned short port,
			  const char *cert_file, const char *key_file)      // 指定 host、port、SSL 证书启动。
	{
		return start(AF_INET, host, port, cert_file, key_file); // 默认 IPv4。
	}

	int start(int family, const char *host, unsigned short port,
			  const char *cert_file, const char *key_file);     // 通用启动接口：地址族、host、port、证书、私钥。

	int start(const struct sockaddr *bind_addr, socklen_t addrlen,
			  const char *cert_file, const char *key_file);     // 最底层启动接口：直接传入 socket 地址。

	int serve(int listen_fd)                                   // serve(listen_fd)：使用已有监听 fd 启动服务。
	{
		return serve(listen_fd, NULL, NULL);                    // 不使用 SSL。
	}

	int serve(int listen_fd, const char *cert_file, const char *key_file); // 使用已有 fd 和 SSL 证书启动。

	void stop()                                                // stop()：阻塞式停止服务。
	{
		this->shutdown();                                      // 先发起非阻塞关闭。
		this->wait_finish();                                   // 再等待服务完全停止。
	}

	void shutdown();                                           // shutdown()：非阻塞关闭服务，不等待完成。
	void wait_finish();                                        // wait_finish()：等待服务关闭和解绑完成。

public:
	size_t get_conn_count() const { return this->conn_count; }  // get_conn_count()：获取当前连接数。

	int get_listen_addr(struct sockaddr *addr, socklen_t *addrlen) const // get_listen_addr()：获取实际监听地址。
	{
		if (this->listen_fd >= 0)                              // 如果服务已经有合法监听 fd。
			return getsockname(this->listen_fd, addr, addrlen); // 调用系统接口获取监听地址。

		errno = ENOTCONN;                                      // 如果未启动监听，设置未连接错误。
		return -1;                                             // 返回失败。
	}

	const struct WFServerParams *get_params() const            // get_params()：获取当前服务端配置。
	{
		return &this->params;                                  // 返回 params 地址，供外部只读查看。
	}

protected:
	virtual SSL_CTX *new_ssl_ctx(const char *cert_file, const char *key_file); // 创建服务端 SSL_CTX，可被子类覆盖。

	virtual SSL_CTX *get_server_ssl_ctx(const char *servername) // get_server_ssl_ctx(servername)：根据 SNI 返回 SSL_CTX。
	{
		return this->get_ssl_ctx();                            // 默认直接返回 CommService 内部 SSL_CTX。
	}

	static int ssl_ctx_callback(SSL *ssl, int *al, void *arg);  // OpenSSL SNI 回调函数，内部会调用 get_server_ssl_ctx。

protected:
	WFServerParams params;                                     // params：当前服务端配置参数。

protected:
	virtual int create_listen_fd();                            // create_listen_fd()：创建监听 fd，可按 transport_type 定制。
	virtual WFConnection *new_connection(int accept_fd);        // new_connection(accept_fd)：新连接到来时创建 WFConnection。
	void delete_connection(WFConnection *conn);                 // delete_connection(conn)：删除连接并维护连接计数。

private:
	int init(const struct sockaddr *bind_addr, socklen_t addrlen,
			 const char *cert_file, const char *key_file);      // init()：内部初始化监听地址、SSL、CommService 等。
	virtual void handle_unbound();                             // handle_unbound()：服务解绑完成时被底层调用。

protected:
	std::atomic<size_t> conn_count;                            // conn_count：当前连接数，原子变量用于多线程安全更新。

private:
	int listen_fd;                                             // listen_fd：监听 socket 文件描述符。
	bool unbind_finish;                                        // unbind_finish：是否已经完成 unbind。

	std::mutex mutex;                                          // mutex：保护停止/等待相关状态。
	std::condition_variable cond;                              // cond：通知 wait_finish 服务已经关闭。

	class CommScheduler *scheduler;                            // scheduler：全局通信调度器，用于 bind/unbind 服务。
};

template<class REQ, class RESP>                               // REQ：请求协议类型；RESP：响应协议类型。
class WFServer : public WFServerBase                           // WFServer：通用协议服务端模板。
{
public:
	WFServer(const struct WFServerParams *params,               // params：服务端自定义配置。
			 std::function<void (WFNetworkTask<REQ, RESP> *)> proc) : // proc：用户请求处理函数。
		WFServerBase(params),                                  // 初始化基础服务端配置。
		process(std::move(proc))                               // 保存用户 process 回调。
	{
	}

	WFServer(std::function<void (WFNetworkTask<REQ, RESP> *)> proc) : // 使用默认配置创建服务端。
		WFServerBase(&SERVER_PARAMS_DEFAULT),                  // 使用通用默认配置。
		process(std::move(proc))                               // 保存用户 process。
	{
	}

protected:
	virtual CommSession *new_session(long long seq, CommConnection *conn); // new_session()：新请求/连接到来时创建通信会话。

protected:
	std::function<void (WFNetworkTask<REQ, RESP> *)> process;   // process：用户业务回调，比如处理 HTTP 请求。
};

template<class REQ, class RESP>
CommSession *WFServer<REQ, RESP>::new_session(long long seq, CommConnection *conn) // 创建服务端会话任务。
{
	using factory = WFNetworkTaskFactory<REQ, RESP>;            // factory：当前协议对应的网络任务工厂。
	WFNetworkTask<REQ, RESP> *task;                             // task：即将创建的服务端网络任务。

	task = factory::create_server_task(this, this->process);    // 创建服务端任务，把当前服务和用户 process 绑定进去。
	task->set_keep_alive(this->params.keep_alive_timeout);      // 设置连接 keep-alive 超时。
	task->set_receive_timeout(this->params.receive_timeout);    // 设置完整接收请求的超时。
	task->get_req()->set_size_limit(this->params.request_size_limit); // 设置请求大小限制。

	return task;                                                // 返回 CommSession 指针，底层 Communicator 会驱动它收请求、发响应。
}

#endif                                                        // 结束头文件保护宏。
