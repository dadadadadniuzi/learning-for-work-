/*
  注释版源码文件：HttpMessage.h

  原始文件位置：
  workflow-master/src/protocol/HttpMessage.h

  本文件用途：
  定义 HTTP 协议对象：
  1. HttpMessage：HTTP 请求和响应的共同基类。
  2. HttpRequest：HTTP 请求对象。
  3. HttpResponse：HTTP 响应对象。
  4. HttpMessageChunk：HTTP chunked 场景下的 chunk 对象。

  对随机图库项目来说：
  - `task->get_req()` 返回 HttpRequest，用来读取 method、URI、body。
  - `task->get_resp()` 返回 HttpResponse，用来设置状态码、header、body。
*/

#ifndef _HTTPMESSAGE_H_                                       // 头文件保护宏：防止重复 include。
#define _HTTPMESSAGE_H_                                       // 定义头文件保护宏。

#include <stdlib.h>                                           // 引入 free 等 C 标准库函数。
#include <string.h>                                           // 引入 strlen 等字符串函数。
#include <utility>                                            // 引入移动语义支持。
#include <string>                                             // 引入 std::string 接口。
#include "list.h"                                             // 引入链表结构，output body 内部用链表保存多个块。
#include "ProtocolMessage.h"                                  // 引入 ProtocolMessage，HTTP 消息要实现 encode/append。
#include "http_parser.h"                                      // 引入底层 HTTP parser。

namespace protocol                                            // protocol 命名空间：所有协议对象都放在这里。
{

struct HttpMessageHeader                                      // HttpMessageHeader：一个 HTTP header 的原始结构。
{
	const void *name;                                          // name：header 名称地址，比如 "Content-Type"。
	size_t name_len;                                           // name_len：header 名称长度。
	const void *value;                                         // value：header 值地址，比如 "text/html"。
	size_t value_len;                                          // value_len：header 值长度。
};

class HttpMessage : public ProtocolMessage                     // HttpMessage：请求和响应的共同父类。
{
public:
	const char *get_http_version() const                       // get_http_version()：获取 HTTP 版本字符串。
	{
		return http_parser_get_version(this->parser);           // 从底层 parser 中读取版本，如 "HTTP/1.1"。
	}

	bool set_http_version(const char *version)                  // set_http_version(version)：设置 HTTP 版本。
	{
		return http_parser_set_version(version, this->parser) == 0; // parser 返回 0 表示设置成功。
	}

	bool is_chunked() const                                    // is_chunked()：判断当前消息是否使用 chunked 传输。
	{
		return http_parser_chunked(this->parser);               // 交给底层 parser 判断 Transfer-Encoding。
	}

	bool is_keep_alive() const                                 // is_keep_alive()：判断当前连接是否 keep-alive。
	{
		return http_parser_keep_alive(this->parser);            // 根据 HTTP 版本和 Connection 头判断。
	}

	bool add_header(const struct HttpMessageHeader *header)     // add_header(header)：添加一个原始 header。
	{
		return http_parser_add_header(header->name, header->name_len,
									  header->value, header->value_len,
									  this->parser) == 0;       // 把 name/value 交给 parser 保存。
	}

	bool add_header_pair(const char *name, const char *value)   // add_header_pair(name, value)：用 C 字符串添加 header。
	{
		return http_parser_add_header(name, strlen(name),
									  value, strlen(value),
									  this->parser) == 0;       // strlen 计算长度后添加。
	}

	bool set_header(const struct HttpMessageHeader *header)     // set_header(header)：设置 header，已有同名 header 会被覆盖。
	{
		return http_parser_set_header(header->name, header->name_len,
									  header->value, header->value_len,
									  this->parser) == 0;
	}

	bool set_header_pair(const char *name, const char *value)   // set_header_pair(name, value)：用 C 字符串设置 header。
	{
		return http_parser_set_header(name, strlen(name),
									  value, strlen(value),
									  this->parser) == 0;
	}

	bool get_parsed_body(const void **body, size_t *size) const // get_parsed_body(body, size)：获取已经解析出来的请求/响应 body。
	{
		return http_parser_get_body(body, size, this->parser) == 0; // body 是输出参数，size 是输出 body 长度。
	}

	bool append_output_body(const void *buf, size_t size);      // append_output_body(buf, size)：追加要发送的 body，会复制数据。

	bool append_output_body(const char *buf)                    // append_output_body(buf)：追加 C 字符串 body。
	{
		return this->append_output_body(buf, strlen(buf));      // 自动用 strlen 计算长度。
	}

	bool append_output_body_nocopy(const void *buf, size_t size); // append_output_body_nocopy：追加 body 但不复制数据。

	bool append_output_body_nocopy(const char *buf)             // nocopy 的 C 字符串版本。
	{
		return this->append_output_body_nocopy(buf, strlen(buf)); // 直接使用传入字符串内存，调用者要保证生命周期。
	}

	size_t get_output_body_size() const                         // get_output_body_size()：获取待发送 body 总大小。
	{
		return this->output_body_size;                          // 返回 output_body_size 成员。
	}

	size_t get_output_body_blocks(const void *buf[], size_t size[],
								  size_t max) const;            // 获取 body 内部块列表，buf/size 为输出数组，max 为最大块数。

	bool get_output_body_merged(void *buf, size_t *size) const; // 把多个 body 块合并复制到用户提供的 buf。

	void clear_output_body();                                  // 清空待发送 body。

public:
	bool get_http_version(std::string& version) const           // std::string 版本：获取 HTTP 版本。
	{
		const char *str = this->get_http_version();             // 先获取 C 字符串版本。

		if (str)                                               // 如果 parser 返回非空。
		{
			version.assign(str);                                // 拷贝到 std::string。
			return true;                                       // 返回成功。
		}

		return false;                                          // 没有版本则返回 false。
	}

	bool set_http_version(const std::string& version)           // std::string 版本：设置 HTTP 版本。
	{
		return this->set_http_version(version.c_str());         // 转成 C 字符串调用。
	}

	bool add_header_pair(const std::string& name, const std::string& value) // std::string 版本：添加 header。
	{
		return http_parser_add_header(name.c_str(), name.size(),
									  value.c_str(), value.size(),
									  this->parser) == 0;
	}

	bool set_header_pair(const std::string& name, const std::string& value) // std::string 版本：设置 header。
	{
		return http_parser_set_header(name.c_str(), name.size(),
									  value.c_str(), value.size(),
									  this->parser) == 0;
	}

	bool append_output_body(const std::string& buf)             // std::string 版本：追加 body 并复制。
	{
		return this->append_output_body(buf.c_str(), buf.size());
	}

	bool append_output_body_nocopy(const std::string& buf)      // std::string 版本：追加 body 不复制。
	{
		return this->append_output_body_nocopy(buf.c_str(), buf.size());
	}

	bool get_output_body_merged(std::string& body) const        // std::string 版本：把输出 body 合并成一个字符串。
	{
		size_t size = this->output_body_size;                   // size：需要合并的总长度。
		body.resize(size);                                      // 先把 string 扩到足够大小。
		return this->get_output_body_merged((void *)body.data(), &size); // 合并写入 string 内部缓冲区。
	}

public:
	bool is_header_complete() const                            // is_header_complete()：判断 header 是否解析完成。
	{
		return http_parser_header_complete(this->parser);       // 调用底层 parser 状态。
	}

	bool has_connection_header() const                          // has_connection_header()：是否存在 Connection 头。
	{
		return http_parser_has_connection(this->parser);
	}

	bool has_content_length_header() const                      // has_content_length_header()：是否存在 Content-Length。
	{
		return http_parser_has_content_length(this->parser);
	}

	bool has_keep_alive_header() const                          // has_keep_alive_header()：是否显式包含 keep-alive。
	{
		return http_parser_has_keep_alive(this->parser);
	}

	void end_parsing()                                         // end_parsing()：告诉 parser 当前消息解析结束。
	{
		http_parser_close_message(this->parser);
	}

	const http_parser_t *get_parser() const                     // get_parser()：返回底层 parser，只读。
	{
		return this->parser;
	}

protected:
	virtual int encode(struct iovec vectors[], int max);        // encode(vectors, max)：把 HTTP 消息编码成 iovec 数组用于发送。
	virtual int append(const void *buf, size_t *size);          // append(buf, size)：把收到的字节追加给 parser 解析。

protected:
	http_parser_t *parser;                                      // parser：底层 HTTP parser 对象。
	size_t cur_size;                                           // cur_size：当前已处理/编码大小，用于内部增量处理。

private:
	struct list_head *combine_from(struct list_head *pos, size_t size); // 内部函数：从某个 body 块开始合并数据。

private:
	struct list_head output_body;                               // output_body：待发送 body 的链表头。
	size_t output_body_size;                                    // output_body_size：待发送 body 总字节数。

public:
	HttpMessage(bool is_resp) : parser(new http_parser_t)       // 构造函数参数 is_resp：true 表示响应，false 表示请求。
	{
		http_parser_init(is_resp, this->parser);                // 初始化底层 parser。
		INIT_LIST_HEAD(&this->output_body);                    // 初始化 output_body 链表。
		this->output_body_size = 0;                             // body 总大小初始化为 0。
		this->cur_size = 0;                                     // 当前处理大小初始化为 0。
	}

	virtual ~HttpMessage()                                      // 析构函数。
	{
		this->clear_output_body();                              // 清理待发送 body 链表。
		if (this->parser)                                      // 如果 parser 不为空。
		{
			http_parser_deinit(this->parser);                   // 释放 parser 内部资源。
			delete this->parser;                                // 删除 parser 对象。
		}
	}

public:
	HttpMessage(HttpMessage&& msg);                             // 移动构造函数。
	HttpMessage& operator = (HttpMessage&& msg);                // 移动赋值函数。
};

class HttpRequest : public HttpMessage                         // HttpRequest：HTTP 请求对象。
{
public:
	const char *get_method() const                              // get_method()：获取请求方法，如 GET/POST。
	{
		return http_parser_get_method(this->parser);
	}

	const char *get_request_uri() const                         // get_request_uri()：获取请求 URI，如 "/" 或 "/image/1"。
	{
		return http_parser_get_uri(this->parser);
	}

	bool set_method(const char *method)                         // set_method(method)：设置请求方法，客户端任务常用。
	{
		return http_parser_set_method(method, this->parser) == 0;
	}

	bool set_request_uri(const char *uri)                       // set_request_uri(uri)：设置请求 URI，客户端任务常用。
	{
		return http_parser_set_uri(uri, this->parser) == 0;
	}

public:
	bool get_method(std::string& method) const                  // std::string 版本：获取 method。
	{
		const char *str = this->get_method();

		if (str)
		{
			method.assign(str);
			return true;
		}

		return false;
	}

	bool get_request_uri(std::string& uri) const                // std::string 版本：获取 URI。
	{
		const char *str = this->get_request_uri();

		if (str)
		{
			uri.assign(str);
			return true;
		}

		return false;
	}

	bool set_method(const std::string& method)                  // std::string 版本：设置 method。
	{
		return this->set_method(method.c_str());
	}

	bool set_request_uri(const std::string& uri)                // std::string 版本：设置 URI。
	{
		return this->set_request_uri(uri.c_str());
	}

protected:
	virtual int append(const void *buf, size_t *size);          // 请求对象的解析逻辑，处理 Expect: 100-continue 等。

private:
	int handle_expect_continue();                              // 内部函数：处理 Expect: 100-continue。

public:
	HttpRequest() : HttpMessage(false) { }                      // 构造请求对象；false 表示不是响应。

public:
	HttpRequest(HttpRequest&& req) = default;                   // 默认移动构造。
	HttpRequest& operator = (HttpRequest&& req) = default;      // 默认移动赋值。
};

class HttpResponse : public HttpMessage                        // HttpResponse：HTTP 响应对象。
{
public:
	const char *get_status_code() const                         // get_status_code()：获取响应状态码，如 "200"。
	{
		return http_parser_get_code(this->parser);
	}

	const char *get_reason_phrase() const                       // get_reason_phrase()：获取原因短语，如 "OK"。
	{
		return http_parser_get_phrase(this->parser);
	}

	bool set_status_code(const char *code)                      // set_status_code(code)：设置响应状态码。
	{
		return http_parser_set_code(code, this->parser) == 0;
	}

	bool set_reason_phrase(const char *phrase)                  // set_reason_phrase(phrase)：设置原因短语。
	{
		return http_parser_set_phrase(phrase, this->parser) == 0;
	}

public:
	bool get_status_code(std::string& code) const               // std::string 版本：获取状态码。
	{
		const char *str = this->get_status_code();

		if (str)
		{
			code.assign(str);
			return true;
		}

		return false;
	}

	bool get_reason_phrase(std::string& phrase) const           // std::string 版本：获取原因短语。
	{
		const char *str = this->get_reason_phrase();

		if (str)
		{
			phrase.assign(str);
			return true;
		}

		return false;
	}

	bool set_status_code(const std::string& code)               // std::string 版本：设置状态码。
	{
		return this->set_status_code(code.c_str());
	}

	bool set_reason_phrase(const std::string& phrase)           // std::string 版本：设置原因短语。
	{
		return this->set_reason_phrase(phrase.c_str());
	}

public:
	void parse_zero_body()                                      // parse_zero_body()：告诉 parser 当前响应没有 body。
	{
		this->parser->transfer_length = 0;                      // 把 transfer_length 设置为 0。
	}

protected:
	virtual int append(const void *buf, size_t *size);          // 响应对象的解析逻辑。

public:
	HttpResponse() : HttpMessage(true) { }                      // 构造响应对象；true 表示响应。

public:
	HttpResponse(HttpResponse&& resp) = default;                // 默认移动构造。
	HttpResponse& operator = (HttpResponse&& resp) = default;   // 默认移动赋值。
};

class HttpMessageChunk : public ProtocolMessage                // HttpMessageChunk：chunked 传输中的一个 chunk。
{
public:
	bool get_chunk_data(const void **chunk_data, size_t *size) const; // 获取 chunk 数据，不转移所有权。
	bool move_chunk_data(void **chunk_data, size_t *size);      // 移出 chunk 数据，转移所有权。
	bool set_chunk_data(const void *chunk_data, size_t size);   // 设置 chunk 数据。

protected:
	virtual int encode(struct iovec vectors[], int max);        // 编码 chunk。
	virtual int append(const void *buf, size_t *size);          // 解析 chunk。

private:
	int append_chunk_line(const void *buf, size_t size);        // 内部函数：解析 chunk 行。

private:
	char chunk_line[32];                                       // chunk_line：保存 chunk size 行。
	void *chunk_data;                                          // chunk_data：chunk 数据缓冲区。
	size_t chunk_size;                                         // chunk_size：chunk 数据大小。
	size_t nreceived;                                          // nreceived：当前已接收字节数。

public:
	HttpMessageChunk()                                         // 构造函数。
	{
		this->chunk_data = NULL;                               // 初始没有 chunk 数据。
		this->nreceived = 0;                                   // 初始已接收字节为 0。
	}

	virtual ~HttpMessageChunk()                                // 析构函数。
	{
		free(this->chunk_data);                                // 释放 chunk_data；它通常由 malloc/realloc 管理。
	}

public:
	HttpMessageChunk(HttpMessageChunk&& msg);                   // 移动构造。
	HttpMessageChunk& operator = (HttpMessageChunk&& msg);      // 移动赋值。
};

}                                                              // namespace protocol 结束。

#endif                                                        // 结束头文件保护宏。
