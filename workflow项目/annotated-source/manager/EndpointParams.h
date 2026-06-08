/*
  注释版源码文件：EndpointParams.h

  原始文件位置：
  workflow-master/src/manager/EndpointParams.h

  本文件用途：
  EndpointParams 用来描述一个网络端点的默认连接参数。

  对随机图库项目的意义：
  - Redis/MySQL/HTTP 客户端任务都会涉及连接超时、响应超时、最大连接数。
  - 面试讲高并发时，max_connections 是非常关键的概念：
    Workflow 不是无限制创建连接，而是按目标端点控制连接数量。
*/

#ifndef _ENDPOINTPARAMS_H_                                     // 头文件保护宏。
#define _ENDPOINTPARAMS_H_                                     // 定义头文件保护宏。

#include <sys/types.h>                                         // 系统类型，例如 size_t。
#include <sys/socket.h>                                        // AF_UNSPEC 等地址族常量。

/**
 * @file   EndpointParams.h
 * @brief  Network config for client task
 */

enum TransportType                                             // TransportType：网络传输类型枚举。
{
	TT_TCP,                                                     // TT_TCP：普通 TCP，HTTP、Redis、MySQL 常用。
	TT_UDP,                                                     // TT_UDP：UDP 传输，DNS 等场景可能使用。
	TT_SCTP,                                                    // TT_SCTP：SCTP 传输类型，较少用于普通 Web 项目。
	TT_TCP_SSL,                                                 // TT_TCP_SSL：基于 TCP 的 TLS/SSL，例如 HTTPS、rediss。
	TT_SCTP_SSL,                                                // TT_SCTP_SSL：基于 SCTP 的 SSL，普通项目基本不用。
};

struct EndpointParams                                          // EndpointParams：单个远端端点的连接配置。
{
	int address_family;                                         // address_family：地址族；AF_UNSPEC 表示 IPv4/IPv6 都可。
	size_t max_connections;                                     // max_connections：该端点允许的最大连接数，用于限制并发连接。
	int connect_timeout;                                        // connect_timeout：TCP 建连超时时间，单位毫秒。
	int response_timeout;                                       // response_timeout：等待响应超时时间，单位毫秒。
	int ssl_connect_timeout;                                    // ssl_connect_timeout：SSL/TLS 握手超时时间，单位毫秒。
	bool use_tls_sni;                                           // use_tls_sni：是否启用 TLS SNI，让服务端按域名选择证书。
};

static constexpr struct EndpointParams ENDPOINT_PARAMS_DEFAULT = // ENDPOINT_PARAMS_DEFAULT：默认端点参数。
{
	.address_family			=	AF_UNSPEC,                       // 默认不限制 IPv4/IPv6。
	.max_connections		=	200,                             // 每个目标端点默认最多 200 条连接。
	.connect_timeout		=	10 * 1000,                       // 建连默认超时 10 秒。
	.response_timeout		=	10 * 1000,                       // 响应默认超时 10 秒。
	.ssl_connect_timeout	=	10 * 1000,                       // SSL 握手默认超时 10 秒。
	.use_tls_sni			=	false,                           // 默认不启用 SNI。
};

#endif                                                         // 结束头文件保护。

