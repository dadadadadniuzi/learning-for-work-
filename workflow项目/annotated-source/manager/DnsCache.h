/*
  注释版源码文件：DnsCache.h

  原始文件位置：
  workflow-master/src/manager/DnsCache.h

  本文件用途：
  DnsCache 是 Workflow 的 DNS 解析结果缓存。

  关键理解：
  - key 是 host + port。
  - value 是 addrinfo + confident_time + expire_time。
  - get 得到的是 handle，不是裸值；用完必须 release。
  - 这样可以避免缓存项被删除时，仍有任务正在使用 addrinfo。

  对随机图库项目的意义：
  - 如果 Redis/MySQL 使用域名，高并发下 DNS 缓存能减少重复解析。
  - 面试讲性能优化时，可以说明 Workflow 对 DNS 也做了 TTL 缓存和引用保护。
*/

#ifndef _DNSCACHE_H_                                          // 头文件保护宏。
#define _DNSCACHE_H_                                          // 定义头文件保护宏。

#include <netdb.h>                                            // addrinfo、freeaddrinfo。
#include <stdint.h>                                           // int64_t。
#include <string>                                             // std::string。
#include <mutex>                                              // std::mutex。
#include <utility>                                            // std::pair。
#include "LRUCache.h"                                         // LRUCache：带 handle 引用的 LRU 缓存。
#include "DnsUtil.h"                                          // protocol::DnsUtil，释放自定义 addrinfo。

#define GET_TYPE_TTL		0                                  // GET_TYPE_TTL：按 expire_time 判断是否可用。
#define GET_TYPE_CONFIDENT	1                                  // GET_TYPE_CONFIDENT：按 confident_time 判断是否足够可信。

struct DnsCacheValue                                          // DnsCacheValue：DNS 缓存值。
{
	struct addrinfo *addrinfo;                                 // addrinfo：DNS 解析结果链表。
	int64_t confident_time;                                    // confident_time：结果“可信”的截止时间。
	int64_t expire_time;                                       // expire_time：结果彻底过期的截止时间。

	bool delayed() const                                       // delayed()：判断这个结果是否是延迟/特殊状态。
	{
		return addrinfo->ai_flags & 2;                         // 通过 ai_flags 的 bit 标记判断。
	}
};

// RAII: NO. Release handle by user
// Thread safety: YES
// MUST call release when handle no longer used
class DnsCache                                                // DnsCache：线程安全 DNS 缓存。
{
public:
	using HostPort = std::pair<std::string, unsigned short>;    // HostPort：缓存 key，由 host 和 port 组成。
	using DnsHandle = LRUHandle<HostPort, DnsCacheValue>;       // DnsHandle：LRUCache 返回的引用句柄。

public:
	// get handler
	// Need call release when handle no longer needed
	//Handle *get(const KEY &key);
	const DnsHandle *get(const HostPort& host_port);            // get(host_port)：按 host+port 获取缓存句柄，用完必须 release。

	const DnsHandle *get(const std::string& host, unsigned short port) // get(host,port)：字符串 host 重载。
	{
		return get(HostPort(host, port));                       // 构造 HostPort 后调用主 get。
	}

	const DnsHandle *get(const char *host, unsigned short port)  // get(char*,port)：C 字符串 host 重载。
	{
		return get(std::string(host), port);                    // 转成 std::string 后复用重载。
	}

	const DnsHandle *get_ttl(const HostPort& host_port)          // get_ttl(host_port)：按 TTL 过期逻辑获取缓存。
	{
		return get_inner(host_port, GET_TYPE_TTL);              // 调用内部 get，并指定 TTL 判断模式。
	}

	const DnsHandle *get_ttl(const std::string& host, unsigned short port) // get_ttl(host,port)：字符串重载。
	{
		return get_ttl(HostPort(host, port));                   // 构造 HostPort 后调用。
	}

	const DnsHandle *get_ttl(const char *host, unsigned short port) // get_ttl(char*,port)：C 字符串重载。
	{
		return get_ttl(std::string(host), port);                // 转成 std::string 后复用。
	}

	const DnsHandle *get_confident(const HostPort& host_port)    // get_confident(host_port)：按 confident_time 获取缓存。
	{
		return get_inner(host_port, GET_TYPE_CONFIDENT);        // 调用内部 get，并指定 confident 判断模式。
	}

	const DnsHandle *get_confident(const std::string& host, unsigned short port) // get_confident(host,port)：字符串重载。
	{
		return get_confident(HostPort(host, port));             // 构造 HostPort 后调用。
	}

	const DnsHandle *get_confident(const char *host, unsigned short port) // get_confident(char*,port)：C 字符串重载。
	{
		return get_confident(std::string(host), port);          // 转成 std::string 后复用。
	}

	const DnsHandle *put(const HostPort& host_port,              // host_port：缓存 key。
						 struct addrinfo *addrinfo,             // addrinfo：要缓存的 DNS 解析结果。
						 unsigned int dns_ttl_default,          // dns_ttl_default：成功结果默认 TTL。
						 unsigned int dns_ttl_min);             // dns_ttl_min：最小 TTL。

	const DnsHandle *put(const std::string& host,                // host：主机名。
						 unsigned short port,                   // port：端口。
						 struct addrinfo *addrinfo,             // addrinfo：解析结果。
						 unsigned int dns_ttl_default,          // dns_ttl_default：默认 TTL。
						 unsigned int dns_ttl_min)              // dns_ttl_min：最小 TTL。
	{
		return put(HostPort(host, port), addrinfo, dns_ttl_default, dns_ttl_min); // 构造 key 后调用主 put。
	}

	const DnsHandle *put(const char *host,                       // host：C 字符串主机名。
						 unsigned short port,                   // port：端口。
						 struct addrinfo *addrinfo,             // addrinfo：解析结果。
						 unsigned int dns_ttl_default,          // dns_ttl_default：默认 TTL。
						 unsigned int dns_ttl_min)              // dns_ttl_min：最小 TTL。
	{
		return put(std::string(host), port, addrinfo, dns_ttl_default, dns_ttl_min); // 转成 std::string 后复用。
	}

	// release handle by get/put
	void release(const DnsHandle *handle);                       // release(handle)：释放 get/put 返回的 handle，非常重要。

	// delete from cache, deleter delay called when all inuse-handle release.
	void del(const HostPort& key);                               // del(key)：从缓存删除 key；若仍被使用，真正释放会延迟。

	void del(const std::string& host, unsigned short port)       // del(host,port)：字符串重载。
	{
		del(HostPort(host, port));                              // 构造 key 后调用主 del。
	}

	void del(const char *host, unsigned short port)              // del(char*,port)：C 字符串重载。
	{
		del(std::string(host), port);                           // 转成 std::string 后复用。
	}

private:
	const DnsHandle *get_inner(const HostPort& host_port, int type); // get_inner(host_port,type)：内部获取函数，type 决定 TTL/可信判断。

	std::mutex mutex_;                                          // mutex_：保护 cache_pool_ 的并发访问。

	class ValueDeleter                                          // ValueDeleter：缓存值释放器。
	{
	public:
		void operator() (const DnsCacheValue& value) const       // operator()(value)：释放一个 DnsCacheValue。
		{
			struct addrinfo *ai = value.addrinfo;                // ai：取出缓存中的 addrinfo 指针。

			if (ai)                                             // 如果 addrinfo 不为空。
			{
				if (ai->ai_flags & 1)                            // 如果 ai_flags 标记它来自系统 getaddrinfo。
					freeaddrinfo(ai);                            // 用系统 freeaddrinfo 释放。
				else                                             // 否则说明它来自 Workflow 自己的 DNS 工具。
					protocol::DnsUtil::freeaddrinfo(ai);         // 用 DnsUtil 的释放函数释放。
			}
		}
	};

	LRUCache<HostPort, DnsCacheValue, ValueDeleter> cache_pool_; // cache_pool_：真正保存 DNS 缓存的 LRUCache。

public:
	// To prevent inline calling LRUCache's constructor and deconstructor.
	DnsCache();                                                 // 构造函数：放到 .cc 中，避免直接内联构造 LRUCache。
	~DnsCache();                                                // 析构函数：放到 .cc 中，避免直接内联析构 LRUCache。
};

#endif                                                        // 结束头文件保护。

