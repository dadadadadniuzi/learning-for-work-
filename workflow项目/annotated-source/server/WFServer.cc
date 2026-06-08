/*
  注释版源码文件：WFServer.cc

  原始文件位置：
  workflow-master/src/server/WFServer.cc

  本文件用途：
  实现 Workflow 服务端基础类的启动、监听、SSL 初始化、连接数控制和关闭等待。

  对随机图库项目最重要：
  - WFHttpServer::start() 最终会走到 WFServerBase::start()。
  - start() 会创建监听地址并绑定到全局 CommScheduler。
  - new_connection() 里有 max_connections 控制，不是无限接收连接。
  - shutdown() / wait_finish() 用于优雅停止服务。
*/

#include <sys/types.h>                                        // 系统类型。
#include <sys/socket.h>                                       // socket、setsockopt、getsockname。
#include <netinet/in.h>                                       // sockaddr_in 等。
#include <netinet/tcp.h>                                      // TCP 相关选项。
#include <errno.h>                                            // errno、EINVAL、EMFILE 等。
#include <unistd.h>                                           // dup。
#include <stdio.h>                                            // snprintf。
#include <atomic>                                             // std::atomic，用于连接数统计。
#include <mutex>                                              // std::mutex。
#include <condition_variable>                                 // std::condition_variable。
#include <openssl/ssl.h>                                      // SSL、SSL_CTX。
#include "CommScheduler.h"                                    // CommScheduler，服务端 bind/unbind。
#include "EndpointParams.h"                                   // TransportType。
#include "WFConnection.h"                                     // WFConnection。
#include "WFGlobal.h"                                         // WFGlobal::get_scheduler/new_ssl_server_ctx。
#include "WFServer.h"                                         // WFServerBase 定义。

#define PORT_STR_MAX	5                                      // 端口字符串最大 5 位，例如 65535。

class WFServerConnection : public WFConnection                 // WFServerConnection：服务端连接对象。
{
public:
	WFServerConnection(std::atomic<size_t> *conn_count)         // 构造函数，传入服务器连接计数器。
	{
		this->conn_count = conn_count;                          // 保存连接计数器指针。
	}

	virtual ~WFServerConnection()                              // 析构函数：连接关闭时调用。
	{
		(*this->conn_count)--;                                  // 连接对象销毁，服务器当前连接数减一。
	}

private:
	std::atomic<size_t> *conn_count;                            // conn_count：指向 WFServerBase 中的连接数。
};

int WFServerBase::ssl_ctx_callback(SSL *ssl, int *al, void *arg) // ssl_ctx_callback：TLS SNI 回调。
{
	WFServerBase *server = (WFServerBase *)arg;                 // arg 是注册回调时传入的 server 指针。
	const char *servername = SSL_get_servername(ssl, TLSEXT_NAMETYPE_host_name); // 获取客户端请求的 SNI 域名。
	SSL_CTX *ssl_ctx = server->get_server_ssl_ctx(servername);  // 根据域名获取对应 SSL_CTX。

	if (!ssl_ctx)                                               // 如果没有找到证书上下文。
		return SSL_TLSEXT_ERR_NOACK;                            // 告诉 OpenSSL 不处理 SNI。

	if (ssl_ctx != server->get_ssl_ctx())                       // 如果找到的不是默认 SSL_CTX。
		SSL_set_SSL_CTX(ssl, ssl_ctx);                          // 切换当前连接使用的 SSL_CTX。

	return SSL_TLSEXT_ERR_OK;                                   // SNI 处理成功。
}

SSL_CTX *WFServerBase::new_ssl_ctx(const char *cert_file, const char *key_file) // new_ssl_ctx：创建服务端 SSL_CTX。
{
	SSL_CTX *ssl_ctx = WFGlobal::new_ssl_server_ctx();          // 从 WFGlobal 创建默认服务端 SSL_CTX。

	if (!ssl_ctx)                                               // 创建失败。
		return NULL;                                            // 返回 NULL。

	if (SSL_CTX_use_certificate_chain_file(ssl_ctx, cert_file) > 0 && // 加载证书链文件。
		SSL_CTX_use_PrivateKey_file(ssl_ctx, key_file, SSL_FILETYPE_PEM) > 0 && // 加载私钥文件。
		SSL_CTX_check_private_key(ssl_ctx) > 0 &&               // 检查私钥和证书是否匹配。
		SSL_CTX_set_tlsext_servername_callback(ssl_ctx, ssl_ctx_callback) > 0 && // 设置 SNI 回调。
		SSL_CTX_set_tlsext_servername_arg(ssl_ctx, this) > 0)   // SNI 回调参数设置为当前 server。
	{
		return ssl_ctx;                                         // SSL_CTX 完整初始化成功。
	}

	SSL_CTX_free(ssl_ctx);                                      // 任一步失败都释放 SSL_CTX。
	return NULL;                                                // 返回失败。
}

int WFServerBase::init(const struct sockaddr *bind_addr, socklen_t addrlen, // bind_addr/addrlen：监听地址。
					   const char *cert_file, const char *key_file)         // cert_file/key_file：TLS 证书和私钥。
{
	int timeout = this->params.peer_response_timeout;            // timeout：服务端 peer 响应超时初始值。

	if (this->params.receive_timeout >= 0)                       // 如果设置了接收超时。
	{
		if ((unsigned int)timeout > (unsigned int)this->params.receive_timeout) // 取更小的超时时间。
			timeout = this->params.receive_timeout;              // 使用 receive_timeout。
	}

	if (this->params.transport_type == TT_TCP_SSL ||             // 如果是 TCP SSL。
		this->params.transport_type == TT_SCTP_SSL)              // 或 SCTP SSL。
	{
		if (!cert_file || !key_file)                             // SSL Server 必须有证书和私钥。
		{
			errno = EINVAL;                                      // 参数无效。
			return -1;                                          // 初始化失败。
		}
	}

	if (this->CommService::init(bind_addr, addrlen, -1, timeout) < 0) // 初始化底层 CommService，包括地址和超时。
		return -1;                                              // 初始化失败。

	if (cert_file && key_file && this->params.transport_type != TT_UDP) // 如果提供证书且不是 UDP。
	{
		SSL_CTX *ssl_ctx = this->new_ssl_ctx(cert_file, key_file); // 创建 SSL_CTX。

		if (!ssl_ctx)                                           // SSL_CTX 创建失败。
		{
			this->deinit();                                     // 清理 CommService。
			return -1;                                          // 返回失败。
		}

		this->set_ssl(ssl_ctx, this->params.ssl_accept_timeout); // 设置服务端 SSL 和握手超时。
	}

	this->scheduler = WFGlobal::get_scheduler();                // 保存全局 CommScheduler，后续 bind/unbind 都用它。
	return 0;                                                   // 初始化成功。
}

int WFServerBase::create_listen_fd()                            // create_listen_fd()：创建监听 socket。
{
	if (this->listen_fd < 0)                                    // 如果没有外部传入监听 fd。
	{
		const struct sockaddr *bind_addr;                       // bind_addr：监听地址。
		socklen_t addrlen;                                      // addrlen：地址长度。
		int type, protocol;                                     // type/protocol：socket 类型和协议。
		int reuse = 1;                                          // reuse：SO_REUSEADDR 开关。

		switch (this->params.transport_type)                    // 根据传输类型选择 socket 类型。
		{
		case TT_TCP:                                            // 普通 TCP。
		case TT_TCP_SSL:                                        // SSL TCP 底层也是 TCP。
			type = SOCK_STREAM;                                 // 流式 socket。
			protocol = 0;                                       // 默认协议。
			break;
		case TT_UDP:                                            // UDP。
			type = SOCK_DGRAM;                                  // 数据报 socket。
			protocol = 0;                                       // 默认协议。
			break;
#ifdef IPPROTO_SCTP
		case TT_SCTP:                                           // SCTP。
		case TT_SCTP_SSL:                                       // SSL SCTP。
			type = SOCK_STREAM;                                 // SCTP 这里用 stream 类型。
			protocol = IPPROTO_SCTP;                            // 指定 SCTP 协议。
			break;
#endif
		default:                                                // 不支持的传输类型。
			errno = EPROTONOSUPPORT;                            // 协议不支持。
			return -1;                                          // 创建失败。
		}

		this->get_addr(&bind_addr, &addrlen);                   // 从 CommService 取监听地址。
		this->listen_fd = socket(bind_addr->sa_family, type, protocol); // 创建 socket。
		if (this->listen_fd >= 0)                               // socket 创建成功。
		{
			setsockopt(this->listen_fd, SOL_SOCKET, SO_REUSEADDR, // 设置地址复用，方便服务重启后快速绑定端口。
					   &reuse, sizeof (int));
		}
	}
	else                                                       // 如果 listen_fd 已经由 serve() 设置。
		this->listen_fd = dup(this->listen_fd);                 // dup 一份 fd，避免直接占用外部 fd 所有权。

	return this->listen_fd;                                     // 返回监听 fd。
}

WFConnection *WFServerBase::new_connection(int accept_fd)       // new_connection(accept_fd)：accept 成功后创建连接对象。
{
	if (++this->conn_count > this->params.max_connections &&     // 先增加连接数，再判断是否超过最大连接数。
		this->drain(1) <= 0)                                    // drain(1) 尝试清理一个连接；失败说明确实满了。
	{
		this->conn_count--;                                     // 超限失败，把计数还原。
		errno = EMFILE;                                         // 用 EMFILE 表示连接资源不足。
		return NULL;                                            // 返回 NULL，拒绝该连接。
	}

	return new WFServerConnection(&this->conn_count);            // 创建服务端连接对象，并让其析构时自动减少计数。
}

void WFServerBase::delete_connection(WFConnection *conn)        // delete_connection(conn)：删除连接对象。
{
	delete (WFServerConnection *)conn;                          // 转回 WFServerConnection 删除，从而触发连接数减少。
}

void WFServerBase::handle_unbound()                             // handle_unbound()：服务解绑完成回调。
{
	this->mutex.lock();                                         // 加锁保护 unbind_finish。
	this->unbind_finish = true;                                 // 标记 unbind 已完成。
	this->cond.notify_one();                                    // 唤醒 wait_finish() 中等待的线程。
	this->mutex.unlock();                                       // 解锁。
}

int WFServerBase::start(const struct sockaddr *bind_addr, socklen_t addrlen, // start：按 sockaddr 启动服务。
						const char *cert_file, const char *key_file)         // cert_file/key_file：可选 SSL 文件。
{
	SSL_CTX *ssl_ctx;                                           // ssl_ctx：失败清理时用。

	if (this->init(bind_addr, addrlen, cert_file, key_file) >= 0) // 初始化服务端对象。
	{
		if (this->scheduler->bind(this) >= 0)                   // 把当前 CommService 绑定到全局 scheduler。
			return 0;                                           // bind 成功，服务启动完成。

		ssl_ctx = this->get_ssl_ctx();                          // bind 失败时取出 SSL_CTX。
		this->deinit();                                         // 清理 CommService。
		if (ssl_ctx)                                            // 如果创建过 SSL_CTX。
			SSL_CTX_free(ssl_ctx);                              // 释放 SSL_CTX。
	}

	this->listen_fd = -1;                                       // 启动失败，重置 listen_fd。
	return -1;                                                  // 返回失败。
}

int WFServerBase::start(int family, const char *host, unsigned short port, // start：按 host/port 启动。
						const char *cert_file, const char *key_file)       // cert_file/key_file：可选 SSL 文件。
{
	struct addrinfo hints = {                                   // hints：getaddrinfo 查询参数。
		.ai_flags		=	AI_PASSIVE,                         // AI_PASSIVE：用于 bind 地址。
		.ai_family		=	family,                             // family：AF_INET/AF_INET6/AF_UNSPEC。
		.ai_socktype	=	SOCK_STREAM,                        // 默认流式 socket。
	};
	struct addrinfo *addrinfo;                                  // addrinfo：getaddrinfo 输出地址。
	char port_str[PORT_STR_MAX + 1];                            // port_str：端口字符串。
	int ret;                                                    // ret：返回值。

	snprintf(port_str, PORT_STR_MAX + 1, "%d", port);           // 把端口数字转成字符串。
	ret = getaddrinfo(host, port_str, &hints, &addrinfo);        // 解析监听 host/port。
	if (ret == 0)                                               // 解析成功。
	{
		ret = start(addrinfo->ai_addr, (socklen_t)addrinfo->ai_addrlen, // 使用解析出的 sockaddr 启动。
					cert_file, key_file);
		freeaddrinfo(addrinfo);                                 // 释放地址结果。
	}
	else                                                        // getaddrinfo 失败。
	{
		if (ret != EAI_SYSTEM)                                  // 如果不是系统 errno 类型错误。
			errno = EINVAL;                                     // 统一设置参数错误。
		ret = -1;                                               // 返回失败。
	}

	return ret;                                                 // 返回启动结果。
}

int WFServerBase::serve(int listen_fd,                          // serve(listen_fd)：使用外部已经创建好的监听 fd。
						const char *cert_file, const char *key_file) // cert_file/key_file：可选 SSL 文件。
{
	struct sockaddr_storage ss;                                 // ss：保存 listen_fd 绑定地址。
	socklen_t len = sizeof ss;                                  // len：地址缓冲区长度。

	if (getsockname(listen_fd, (struct sockaddr *)&ss, &len) < 0) // 获取 listen_fd 当前绑定地址。
		return -1;                                              // 获取失败。

	this->listen_fd = listen_fd;                                // 保存外部 fd。
	return start((struct sockaddr *)&ss, len, cert_file, key_file); // 按该地址启动服务。
}

void WFServerBase::shutdown()                                   // shutdown()：停止接受新连接。
{
	this->listen_fd = -1;                                       // 标记监听 fd 不再可用。
	this->scheduler->unbind(this);                              // 从全局 scheduler 解绑当前服务。
}

void WFServerBase::wait_finish()                                // wait_finish()：等待 unbind 完成并清理资源。
{
	SSL_CTX *ssl_ctx = this->get_ssl_ctx();                     // 先取出 SSL_CTX，deinit 后可能不可取。
	std::unique_lock<std::mutex> lock(this->mutex);              // 加锁，准备等待条件变量。

	while (!this->unbind_finish)                                // 只要 unbind 未完成。
		this->cond.wait(lock);                                  // 阻塞等待 handle_unbound 唤醒。

	this->deinit();                                             // unbind 完成后清理 CommService。
	this->unbind_finish = false;                                // 重置标记，允许以后再次 start/shutdown。
	lock.unlock();                                              // 解锁。
	if (ssl_ctx)                                                // 如果有 SSL_CTX。
		SSL_CTX_free(ssl_ctx);                                  // 释放 SSL_CTX。
}

