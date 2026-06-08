/*
  注释版源码文件：poller.h

  原始文件位置：
  workflow-master/src/kernel/poller.h

  本文件用途：
  poller 是 Workflow 底层事件循环抽象。

  面试理解：
  1. socket 读写、connect、listen、timer 都被抽象成 poller_data。
  2. poller 监听事件完成后，通过 poller_result 回调给 Communicator。
  3. Linux 下具体实现通常基于 epoll/timerfd 等机制。
*/

#ifndef _POLLER_H_                                            // 头文件保护宏。
#define _POLLER_H_                                            // 定义头文件保护宏。

#include <sys/types.h>                                        // 系统类型。
#include <sys/socket.h>                                       // sockaddr/socklen_t。
#include <time.h>                                             // timespec。
#include <openssl/ssl.h>                                      // SSL。

typedef struct __poller poller_t;                             // poller_t：poller 对象的不透明类型。
typedef struct __poller_message poller_message_t;              // poller_message_t：poller 接收消息对象。

struct __poller_message                                       // __poller_message：用于增量接收协议消息。
{
	int (*append)(const void *, size_t *, poller_message_t *);  // append：把收到的数据追加进消息解析器。
	char data[0];                                              // data：柔性数组，占位具体消息对象的附加内存。
};

struct poller_data                                            // poller_data：一个要注册到 poller 的事件描述。
{
#define PD_OP_TIMER			0                                  // 定时器事件。
#define PD_OP_READ			1                                  // 普通读事件。
#define PD_OP_WRITE			2                                  // 普通写事件。
#define PD_OP_LISTEN		3                                  // 监听/accept 事件。
#define PD_OP_CONNECT		4                                  // 非阻塞 connect 事件。
#define PD_OP_RECVFROM		5                                  // UDP recvfrom 事件。
#define PD_OP_SSL_READ		PD_OP_READ                         // SSL 读复用 READ。
#define PD_OP_SSL_WRITE		PD_OP_WRITE                        // SSL 写复用 WRITE。
#define PD_OP_SSL_ACCEPT	6                                  // SSL accept 握手。
#define PD_OP_SSL_CONNECT	7                                  // SSL connect 握手。
#define PD_OP_SSL_SHUTDOWN	8                                  // SSL shutdown。
#define PD_OP_EVENT			9                                  // 自定义事件。
#define PD_OP_NOTIFY		10                                 // 通知事件。
	short operation;                                           // operation：当前事件类型。
	unsigned short iovcnt;                                     // iovcnt：写事件中 iovec 数量。
	int fd;                                                    // fd：要监听的文件描述符。
	SSL *ssl;                                                  // ssl：SSL 连接对象，非 SSL 时为空。
	union
	{
		poller_message_t *(*create_message)(void *);            // create_message：读事件开始时创建消息解析对象。
		int (*partial_written)(size_t, void *);                 // partial_written：部分写入回调。
		void *(*accept)(const struct sockaddr *, socklen_t, int, void *); // accept 回调。
		void *(*recvfrom)(const struct sockaddr *, socklen_t,
						  const void *, size_t, void *);       // recvfrom 回调。
		void *(*event)(void *);                                // 自定义事件回调。
		void *(*notify)(void *, void *);                       // notify 回调。
	};
	void *context;                                             // context：用户上下文，Communicator 通常放连接条目。
	union
	{
		poller_message_t *message;                             // message：读事件使用的消息解析对象。
		struct iovec *write_iov;                               // write_iov：写事件使用的 iovec 数组。
		void *result;                                          // result：事件完成后的结果指针。
	};
};

struct poller_result                                          // poller_result：poller 事件完成后的结果。
{
#define PR_ST_SUCCESS		0                                  // 成功。
#define PR_ST_FINISHED		1                                  // 完成。
#define PR_ST_ERROR			2                                  // 错误。
#define PR_ST_DELETED		3                                  // 事件被删除。
#define PR_ST_MODIFIED		4                                  // 事件被修改。
#define PR_ST_STOPPED		5                                  // poller 停止。
	int state;                                                 // state：事件结果状态。
	int error;                                                 // error：错误码。
	struct poller_data data;                                   // data：原事件数据，包含 fd、operation、context 等。
};

struct poller_params                                          // poller_params：创建 poller 时的参数。
{
	size_t max_open_files;                                     // max_open_files：最大打开文件数，用于内部容量规划。
	void (*callback)(struct poller_result *, void *);           // callback：事件完成后的回调入口。
	void *context;                                             // context：传给 callback 的全局上下文。
};

#ifdef __cplusplus
extern "C"                                                    // C++ 编译时使用 C 链接，方便 C/C++ 混用。
{
#endif

poller_t *poller_create(const struct poller_params *params);   // 创建 poller。
int poller_start(poller_t *poller);                            // 启动 poller 事件循环线程。
int poller_add(const struct poller_data *data, int timeout, poller_t *poller); // 添加 fd 事件。
int poller_del(int fd, poller_t *poller);                      // 删除 fd 事件。
int poller_mod(const struct poller_data *data, int timeout, poller_t *poller); // 修改 fd 事件。
int poller_set_timeout(int fd, int timeout, poller_t *poller); // 修改 fd 超时。
int poller_add_timer(const struct timespec *value, void *context, void **timer,
					 poller_t *poller);                        // 添加定时器。
int poller_del_timer(void *timer, poller_t *poller);           // 删除定时器。
void poller_set_callback(void (*callback)(struct poller_result *, void *),
						 poller_t *poller);                    // 设置 poller 回调。
void poller_stop(poller_t *poller);                            // 停止 poller。
void poller_destroy(poller_t *poller);                         // 销毁 poller。

#ifdef __cplusplus
}
#endif

#endif                                                        // 结束头文件保护宏。
