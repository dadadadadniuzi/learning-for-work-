/*
  注释版源码文件：WFHttpServer.h

  原始文件位置：
  workflow-master/src/server/WFHttpServer.h

  本文件用途：
  定义 HTTP 服务端类型 WFHttpServer。

  对随机图库项目来说：
  你写的代码大概是：

      WFHttpServer server([](WFHttpTask *task) {
          // 读取 task->get_req()
          // 创建 Redis/MySQL/FileIO 任务
          // 最后写 task->get_resp()
      });

  这里的 WFHttpServer 本质上是：
      WFServer<protocol::HttpRequest, protocol::HttpResponse>
*/

#ifndef _WFHTTPSERVER_H_                                      // 头文件保护宏：防止重复 include。
#define _WFHTTPSERVER_H_                                      // 定义头文件保护宏。

#include <utility>                                            // 引入 std::move，用于移动 process 回调。
#include "HttpMessage.h"                                      // 引入 HttpRequest 和 HttpResponse。
#include "WFServer.h"                                         // 引入通用 WFServer 模板。
#include "WFTaskFactory.h"                                    // 引入 WFServerTaskFactory / WFHttpTask 等任务工厂。

using http_process_t = std::function<void (WFHttpTask *)>;    // http_process_t：HTTP 服务端处理函数类型。
                                                               // 输入参数 WFHttpTask*：一次 HTTP 请求对应的任务对象。

using WFHttpServer = WFServer<protocol::HttpRequest,
							  protocol::HttpResponse>;        // WFHttpServer：把通用 WFServer 特化成 HTTP 请求/响应类型。

static constexpr struct WFServerParams HTTP_SERVER_PARAMS_DEFAULT = // HTTP 服务端默认参数。
{
	.transport_type			=	TT_TCP,                         // HTTP 默认基于 TCP。
	.max_connections		=	2000,                           // 默认最大连接数 2000。
	.peer_response_timeout	=	10 * 1000,                     // 单次读写对端超时 10 秒。
	.receive_timeout		=	-1,                             // 接收完整请求默认不设总超时。
	.keep_alive_timeout		=	60 * 1000,                     // HTTP keep-alive 默认 60 秒。
	.request_size_limit		=	(size_t)-1,                    // 请求大小默认不限制。
	.ssl_accept_timeout		=	10 * 1000,                     // HTTPS SSL 握手超时 10 秒。
};

template<> inline                                             // 模板特化：专门定义 WFHttpServer 的构造函数。
WFHttpServer::WFServer(http_process_t proc) :                 // 构造函数参数 proc：用户写的 HTTP 处理回调。
	WFServerBase(&HTTP_SERVER_PARAMS_DEFAULT),                 // 用 HTTP 默认参数初始化服务端基础类。
	process(std::move(proc))                                   // 保存用户 process 回调；std::move 避免复制 std::function。
{
}

template<> inline                                             // 模板特化：专门定义 HTTP 服务端如何创建新会话。
CommSession *WFHttpServer::new_session(long long seq, CommConnection *conn)
{                                                              // 输入参数 seq：连接/请求序号，当前函数内没有直接使用。
	                                                           // 输入参数 conn：底层连接对象，当前函数内没有直接使用。
	WFHttpTask *task;                                          // task：即将创建的 HTTP 服务端任务。

	task = WFServerTaskFactory::create_http_task(this, this->process); // 创建 HTTP 服务端任务，并绑定当前 server 和用户 process。
	task->set_keep_alive(this->params.keep_alive_timeout);     // 设置 HTTP keep-alive 超时时间。
	task->set_receive_timeout(this->params.receive_timeout);   // 设置接收完整 HTTP 请求的超时时间。
	task->get_req()->set_size_limit(this->params.request_size_limit); // 设置 HTTP 请求大小限制，防止过大请求。

	return task;                                               // 返回任务作为 CommSession，底层通信层会负责收请求、触发 process、发响应。
}

#endif                                                        // 结束头文件保护宏。
