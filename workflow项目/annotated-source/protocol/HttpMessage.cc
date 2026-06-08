/*
  注释版源码文件：HttpMessage.cc

  原始文件位置：
  workflow-master/src/protocol/HttpMessage.cc

  本文件用途：
  实现 HTTP 消息的 body 管理、编码发送、接收解析和 chunk 处理。

  对随机图库项目最重要的点：
  1. HttpResponse::append_output_body() 如何把 HTML/图片数据挂到响应 body。
  2. HttpMessage::encode() 如何把状态行、Header、Body 拼成 iovec 交给底层发送。
  3. HttpRequest::append() 如何解析浏览器发来的 HTTP 请求。
  4. size_limit 如何限制请求体大小，防止过大的上传请求打爆内存。
*/

#include <errno.h>                                             // errno、ENOSPC、EBADMSG、EMSGSIZE 等错误码。
#include <stdlib.h>                                            // malloc/free。
#include <stdio.h>                                             // sprintf。
#include <string.h>                                            // memcpy、strlen、strcmp。
#include <utility>                                             // std::move。
#include "HttpMessage.h"                                      // HttpMessage、HttpRequest、HttpResponse 定义。

namespace protocol                                             // HTTP 协议类都放在 protocol 命名空间里。
{

struct HttpMessageBlock                                        // HttpMessageBlock：HTTP 输出 body 的一个数据块。
{
	struct list_head list;                                      // list：把多个 body 块串成链表。
	const void *ptr;                                            // ptr：指向实际 body 数据。
	size_t size;                                                // size：当前 body 块大小。
};

bool HttpMessage::append_output_body(const void *buf, size_t size) // append_output_body(buf,size)：拷贝一段数据到 HTTP 输出 body。
{
	size_t n = sizeof (struct HttpMessageBlock) + size;         // n：一次性分配“块头 + 数据内容”的总字节数。
	struct HttpMessageBlock *block = (struct HttpMessageBlock *)malloc(n); // block：新 body 块。

	if (block)                                                  // malloc 成功才可以写入。
	{
		memcpy(block + 1, buf, size);                           // 把用户传入数据复制到 block 后面的内存。
		block->ptr = block + 1;                                 // ptr 指向复制后的数据区域。
		block->size = size;                                     // 保存当前块长度。
		list_add_tail(&block->list, &this->output_body);        // 把块追加到 output_body 链表尾部，保持 body 顺序。
		this->output_body_size += size;                         // 累加总输出 body 大小。
		return true;                                            // 返回 true 表示追加成功。
	}

	return false;                                               // malloc 失败，返回 false。
}

bool HttpMessage::append_output_body_nocopy(const void *buf, size_t size) // append_output_body_nocopy：不拷贝数据，只保存外部指针。
{
	size_t n = sizeof (struct HttpMessageBlock);                 // 只需要分配块头，不分配数据副本。
	struct HttpMessageBlock *block = (struct HttpMessageBlock *)malloc(n); // block：body 块头。

	if (block)                                                  // 分配成功。
	{
		block->ptr = buf;                                       // 直接保存外部数据指针；调用者必须保证数据生命周期足够长。
		block->size = size;                                     // 保存数据长度。
		list_add_tail(&block->list, &this->output_body);        // 加入 body 链表。
		this->output_body_size += size;                         // 累加 body 总大小。
		return true;                                            // 返回成功。
	}

	return false;                                               // 分配块头失败。
}

size_t HttpMessage::get_output_body_blocks(const void *buf[], size_t size[], // buf/size：输出每个 body 块的指针和长度数组。
										   size_t max) const                 // max：最多返回多少个块。
{
	struct HttpMessageBlock *block;                              // block：当前遍历到的 body 块。
	struct list_head *pos;                                       // pos：链表遍历游标。
	size_t n = 0;                                                // n：已经输出的块数量。

	list_for_each(pos, &this->output_body)                       // 遍历 output_body 链表。
	{
		if (n == max)                                           // 如果输出数组已满。
			break;                                              // 停止遍历。

		block = list_entry(pos, struct HttpMessageBlock, list);  // 从 list_head 还原出 HttpMessageBlock。
		buf[n] = block->ptr;                                    // 记录第 n 个块的数据指针。
		size[n] = block->size;                                  // 记录第 n 个块的数据大小。
		n++;                                                    // 已输出块数加一。
	}

	return n;                                                   // 返回实际输出的块数量。
}

bool HttpMessage::get_output_body_merged(void *buf, size_t *size) const // get_output_body_merged：把多个 body 块合并复制到一块连续内存。
{
	struct HttpMessageBlock *block;                              // block：当前 body 块。
	struct list_head *pos;                                       // pos：链表游标。

	if (*size < this->output_body_size)                          // 如果调用者给的缓冲区不够大。
	{
		errno = ENOSPC;                                         // 设置空间不足错误。
		return false;                                           // 合并失败。
	}

	list_for_each(pos, &this->output_body)                       // 遍历所有 body 块。
	{
		block = list_entry(pos, struct HttpMessageBlock, list);  // 取出当前块。
		memcpy(buf, block->ptr, block->size);                    // 把当前块复制到目标连续缓冲区。
		buf = (char *)buf + block->size;                         // 目标指针后移。
	}

	*size = this->output_body_size;                              // 输出实际合并后的大小。
	return true;                                                // 合并成功。
}

void HttpMessage::clear_output_body()                           // clear_output_body()：清空输出 body 链表。
{
	struct HttpMessageBlock *block;                              // block：待释放的 body 块。
	struct list_head *pos, *tmp;                                 // pos/tmp：安全遍历用游标，删除时需要 tmp。

	list_for_each_safe(pos, tmp, &this->output_body)             // 安全遍历链表，允许遍历中删除节点。
	{
		block = list_entry(pos, struct HttpMessageBlock, list);  // 取出当前 body 块。
		list_del(pos);                                          // 从链表移除。
		free(block);                                            // 释放块头和可能紧跟其后的拷贝数据。
	}

	this->output_body_size = 0;                                  // body 总大小归零。
}

struct list_head *HttpMessage::combine_from(struct list_head *pos, size_t size) // combine_from(pos,size)：从 pos 开始合并多个 body 块。
{
	size_t n = sizeof (struct HttpMessageBlock) + size;          // n：新合并块需要的总内存。
	struct HttpMessageBlock *block = (struct HttpMessageBlock *)malloc(n); // block：新合并块。
	struct HttpMessageBlock *entry;                              // entry：被合并的旧块。
	char *ptr;                                                   // ptr：写入新块数据的游标。

	if (block)                                                  // 分配新块成功。
	{
		block->ptr = block + 1;                                 // 新块数据放在块头后面。
		block->size = size;                                     // 新块大小等于剩余所有待合并块大小。
		ptr = (char *)block->ptr;                               // 初始化写入游标。

		do                                                       // 从 pos 开始，把后续块全部合并到新块。
		{
			entry = list_entry(pos, struct HttpMessageBlock, list); // 取出旧块。
			pos = pos->next;                                    // 先保存下一个位置。
			list_del(&entry->list);                             // 从链表删除旧块。
			memcpy(ptr, entry->ptr, entry->size);                // 把旧块数据复制进新块。
			ptr += entry->size;                                 // 写入游标后移。
			free(entry);                                        // 释放旧块。
		} while (pos != &this->output_body);                    // 直到链表尾哨兵。

		list_add_tail(&block->list, &this->output_body);        // 把合并后的新块放回链表尾部。
		return &block->list;                                    // 返回新块节点位置。
	}

	return NULL;                                                // 分配失败返回 NULL。
}

int HttpMessage::encode(struct iovec vectors[], int max)        // encode(vectors,max)：把 HTTP 消息编码成 iovec 数组。
{
	const char *start_line[3];                                  // start_line：请求行或响应状态行的 3 个部分。
	http_header_cursor_t cursor;                                // cursor：遍历 header 的游标。
	struct HttpMessageHeader header;                            // header：当前遍历到的 header。
	struct HttpMessageBlock *block;                             // block：当前 body 块。
	struct list_head *pos;                                      // pos：body 链表游标。
	size_t size;                                                // size：剩余 body 总大小，用于必要时合并。
	int i;                                                       // i：当前写入第几个 iovec。

	start_line[0] = http_parser_get_method(this->parser);        // 如果是请求，取 method，例如 GET/POST。
	if (start_line[0])                                          // method 存在说明这是 HTTP Request。
	{
		start_line[1] = http_parser_get_uri(this->parser);       // 请求行第二段是 URI。
		start_line[2] = http_parser_get_version(this->parser);   // 请求行第三段是 HTTP 版本。
	}
	else                                                        // 否则按 Response 状态行处理。
	{
		start_line[0] = http_parser_get_version(this->parser);   // 响应行第一段是 HTTP 版本。
		start_line[1] = http_parser_get_code(this->parser);      // 响应行第二段是状态码。
		start_line[2] = http_parser_get_phrase(this->parser);    // 响应行第三段是状态短语。
	}

	if (!start_line[0] || !start_line[1] || !start_line[2])      // 三段缺一不可。
	{
		errno = EBADMSG;                                        // 设置坏消息错误。
		return -1;                                             // 编码失败。
	}

	vectors[0].iov_base = (void *)start_line[0];                 // iovec 0：请求方法或 HTTP 版本。
	vectors[0].iov_len = strlen(start_line[0]);                  // iovec 0 长度。
	vectors[1].iov_base = (void *)" ";                           // iovec 1：空格。
	vectors[1].iov_len = 1;                                      // 空格长度为 1。

	vectors[2].iov_base = (void *)start_line[1];                 // iovec 2：URI 或状态码。
	vectors[2].iov_len = strlen(start_line[1]);                  // iovec 2 长度。
	vectors[3].iov_base = (void *)" ";                           // iovec 3：空格。
	vectors[3].iov_len = 1;                                      // 空格长度为 1。

	vectors[4].iov_base = (void *)start_line[2];                 // iovec 4：版本或状态短语。
	vectors[4].iov_len = strlen(start_line[2]);                  // iovec 4 长度。
	vectors[5].iov_base = (void *)"\r\n";                        // iovec 5：行结束符。
	vectors[5].iov_len = 2;                                      // CRLF 长度为 2。

	i = 6;                                                       // 前 6 个 iovec 已经用于起始行。
	http_header_cursor_init(&cursor, this->parser);              // 初始化 header 遍历游标。
	while (http_header_cursor_next(&header.name, &header.name_len, // 逐个取 header 名称和值。
								   &header.value, &header.value_len,
								   &cursor) == 0)
	{
		if (i == max)                                           // 如果 iovec 数组满了。
			break;                                              // 停止写入。

		vectors[i].iov_base = (void *)header.name;              // header 在 parser 内部通常是一段连续的 "Name: Value\r\n"。
		vectors[i].iov_len = header.name_len + 2 + header.value_len + 2; // name + ": " + value + "\r\n" 的长度。
		i++;                                                    // iovec 数量加一。
	}

	http_header_cursor_deinit(&cursor);                          // 释放 header 遍历游标资源。
	if (i + 1 >= max)                                            // 至少还需要一个空行 iovec。
	{
		errno = EOVERFLOW;                                      // iovec 数组不够。
		return -1;                                             // 编码失败。
	}

	vectors[i].iov_base = (void *)"\r\n";                        // header 结束空行。
	vectors[i].iov_len = 2;                                      // 空行长度为 2。
	i++;                                                        // iovec 后移。

	size = this->output_body_size;                               // 剩余 body 总大小。
	list_for_each(pos, &this->output_body)                       // 遍历输出 body 块。
	{
		if (i + 1 == max && pos != this->output_body.prev)       // 如果 iovec 只剩一个位置，但 body 还有多个块。
		{
			pos = this->combine_from(pos, size);                 // 合并剩余 body 块，保证能放进 iovec 数组。
			if (!pos)                                           // 合并失败。
				return -1;                                      // 返回失败，errno 由 malloc 相关路径体现。
		}

		block = list_entry(pos, struct HttpMessageBlock, list);  // 取出当前 body 块。
		vectors[i].iov_base = (void *)block->ptr;                // iovec 指向 body 数据。
		vectors[i].iov_len = block->size;                        // iovec 长度为 body 块大小。
		size -= block->size;                                    // 剩余 body 大小减少。
		i++;                                                    // iovec 数量加一。
	}

	return i;                                                    // 返回实际使用的 iovec 数量。
}

inline int HttpMessage::append(const void *buf, size_t *size)   // append(buf,size*)：把收到的网络字节追加给 HTTP parser。
{
	int ret = http_parser_append_message(buf, size, this->parser); // 调用底层 HTTP parser；size 会变成实际消费字节数。

	if (ret >= 0)                                                // ret >= 0 表示解析过程没有协议错误。
	{
		this->cur_size += *size;                                 // 累加当前已接收字节数。
		if (this->cur_size > this->size_limit)                   // 如果超过消息大小限制。
		{
			errno = EMSGSIZE;                                    // 设置消息过大错误。
			ret = -1;                                           // 解析失败。
		}
	}
	else if (ret == -2)                                          // 底层 parser 用 -2 表示 HTTP 格式错误。
	{
		errno = EBADMSG;                                        // 转换成标准坏消息错误。
		ret = -1;                                               // 对外统一返回 -1。
	}

	return ret;                                                  // 返回解析结果：0 未完成，>0 完成，-1 错误。
}

HttpMessage::HttpMessage(HttpMessage&& msg) :                  // HttpMessage 移动构造函数。
	ProtocolMessage(std::move(msg))                             // 先移动父类 ProtocolMessage 部分。
{
	this->parser = msg.parser;                                  // 接管 parser 指针。
	msg.parser = NULL;                                          // 清空源对象，避免重复释放。

	INIT_LIST_HEAD(&this->output_body);                         // 初始化当前对象 body 链表。
	list_splice_init(&msg.output_body, &this->output_body);      // 把源对象 body 链表移动过来，并清空源链表。
	this->output_body_size = msg.output_body_size;               // 接管 body 总大小。
	msg.output_body_size = 0;                                    // 源对象大小归零。

	this->cur_size = msg.cur_size;                               // 接管已解析大小。
	msg.cur_size = 0;                                            // 源对象归零。
}

HttpMessage& HttpMessage::operator = (HttpMessage&& msg)        // HttpMessage 移动赋值函数。
{
	if (&msg != this)                                           // 防止自我移动赋值。
	{
		*(ProtocolMessage *)this = std::move(msg);               // 移动父类部分。

		if (this->parser)                                       // 如果当前对象已有 parser。
		{
			http_parser_deinit(this->parser);                    // 先反初始化 parser。
			delete this->parser;                                // 再释放 parser 对象。
		}

		this->parser = msg.parser;                              // 接管源对象 parser。
		msg.parser = NULL;                                      // 清空源对象 parser。

		this->clear_output_body();                              // 清理当前对象已有 body。
		list_splice_init(&msg.output_body, &this->output_body);  // 移动源对象 body 链表。
		this->output_body_size = msg.output_body_size;           // 接管 body 大小。
		msg.output_body_size = 0;                                // 源对象 body 大小归零。

		this->cur_size = msg.cur_size;                           // 接管当前解析大小。
		msg.cur_size = 0;                                        // 源对象归零。
	}

	return *this;                                                // 返回当前对象引用。
}

#define HTTP_100_STATUS_LINE	"HTTP/1.1 100 Continue"         // 100 Continue 状态行。
#define HTTP_400_STATUS_LINE	"HTTP/1.1 400 Bad Request"      // 400 Bad Request 状态行。
#define HTTP_413_STATUS_LINE	"HTTP/1.1 413 Request Entity Too Large" // 413 请求体过大状态行。
#define HTTP_417_STATUS_LINE	"HTTP/1.1 417 Expectation Failed" // 417 Expect 失败状态行。
#define CONTENT_LENGTH_ZERO		"Content-Length: 0"             // 空响应体 header。
#define CONNECTION_CLOSE		"Connection: close"             // 关闭连接 header。
#define CRLF					"\r\n"                         // HTTP 行结束符。

#define HTTP_100_RESP			HTTP_100_STATUS_LINE CRLF \
								CRLF                           // 100 Continue 完整响应。
#define HTTP_400_RESP			HTTP_400_STATUS_LINE CRLF \
								CONTENT_LENGTH_ZERO CRLF \
								CONNECTION_CLOSE CRLF \
								CRLF                           // 400 错误响应。
#define HTTP_413_RESP			HTTP_413_STATUS_LINE CRLF \
								CONTENT_LENGTH_ZERO CRLF \
								CONNECTION_CLOSE CRLF \
								CRLF                           // 413 错误响应。
#define HTTP_417_RESP			HTTP_417_STATUS_LINE CRLF \
								CONTENT_LENGTH_ZERO CRLF \
								CONNECTION_CLOSE CRLF \
								CRLF                           // 417 错误响应。

int HttpRequest::handle_expect_continue()                      // handle_expect_continue()：处理客户端 Expect: 100-continue。
{
	size_t trans_len = this->parser->transfer_length;            // trans_len：请求体传输长度。
	int ret;                                                     // ret：feedback 写回结果。

	if (trans_len != (size_t)-1)                                 // 如果请求体长度已知。
	{
		if (this->parser->header_offset + trans_len > this->size_limit) // header + body 超过限制。
		{
			this->feedback(HTTP_417_RESP, strlen(HTTP_417_RESP)); // 直接反馈 417。
			errno = EMSGSIZE;                                  // 设置消息过大错误。
			return -1;                                         // 处理失败。
		}
	}

	ret = this->feedback(HTTP_100_RESP, strlen(HTTP_100_RESP));  // 向客户端反馈 100 Continue，允许继续发送 body。
	if (ret != strlen(HTTP_100_RESP))                            // 如果反馈字节数不完整。
	{
		if (ret >= 0)                                           // ret>=0 但长度不够说明缓冲不足。
			errno = ENOBUFS;                                    // 设置缓冲不足。
		return -1;                                             // 返回失败。
	}

	return 0;                                                    // 处理成功。
}

int HttpRequest::append(const void *buf, size_t *size)          // HttpRequest::append：解析 HTTP 请求。
{
	int ret = HttpMessage::append(buf, size);                    // 先调用通用 HTTP 解析逻辑。

	if (ret == 0)                                                // ret==0 表示消息还没完整。
	{
		if (this->parser->expect_continue &&                     // 如果请求带 Expect: 100-continue。
			http_parser_header_complete(this->parser))           // 并且 header 已经解析完成。
		{
			this->parser->expect_continue = 0;                   // 清除 expect 标记，避免重复处理。
			ret = this->handle_expect_continue();                // 回 100 Continue 或错误响应。
		}
	}
	else if (ret < 0)                                            // 解析失败。
	{
		if (errno == EBADMSG)                                    // HTTP 格式错误。
			this->feedback(HTTP_400_RESP, strlen(HTTP_400_RESP)); // 反馈 400。
		else if (errno == EMSGSIZE)                              // 请求体过大。
			this->feedback(HTTP_413_RESP, strlen(HTTP_413_RESP)); // 反馈 413。
	}

	return ret;                                                  // 返回请求解析状态。
}

int HttpResponse::append(const void *buf, size_t *size)         // HttpResponse::append：解析 HTTP 响应。
{
	int ret = HttpMessage::append(buf, size);                    // 调用通用 HTTP 解析。

	if (ret > 0)                                                 // 如果解析到完整响应。
	{
		if (strcmp(http_parser_get_code(this->parser), "100") == 0) // 如果是中间响应 100 Continue。
		{
			http_parser_deinit(this->parser);                    // 清理当前 parser 状态。
			http_parser_init(1, this->parser);                   // 重新初始化为响应 parser。
			ret = 0;                                            // 告诉上层还要继续等真正响应。
		}
	}

	return ret;                                                  // 返回解析状态。
}

bool HttpMessageChunk::get_chunk_data(const void **data, size_t *size) const // 获取完整 chunk 数据。
{
	if (this->chunk_data && this->nreceived == this->chunk_size + 2) // chunk_data 存在且已经收完数据和 CRLF。
	{
		*data = this->chunk_data;                                // 输出 chunk 数据指针。
		*size = this->chunk_size;                                // 输出真实数据大小，不含末尾 CRLF。
		return true;                                            // 获取成功。
	}
	else
		return false;                                           // chunk 尚未完整。
}

bool HttpMessageChunk::move_chunk_data(void **data, size_t *size) // move_chunk_data：把 chunk 数据所有权移交给调用者。
{
	if (this->chunk_data && this->nreceived == this->chunk_size + 2) // 只有完整 chunk 才能移动。
	{
		*data = this->chunk_data;                                // 输出数据指针。
		*size = this->chunk_size;                                // 输出数据大小。
		this->chunk_data = NULL;                                 // 清空内部指针，避免析构时释放。
		this->nreceived = 0;                                     // 接收计数归零。
		return true;                                            // 移动成功。
	}
	else
		return false;                                           // 不完整，不能移动。
}

bool HttpMessageChunk::set_chunk_data(const void *data, size_t size) // set_chunk_data：设置要发送的一个 chunk。
{
	char *p = (char *)malloc(size + 3);                          // 分配数据大小 + "\r\n" + '\0'。

	if (p)                                                       // 分配成功。
	{
		memcpy(p, data, size);                                  // 复制 chunk 数据。
		p[size] = '\r';                                         // 添加 CR。
		p[size + 1] = '\n';                                     // 添加 LF。
		p[size + 2] = '\0';                                     // 添加字符串终止符，方便调试。

		free(this->chunk_data);                                 // 释放旧 chunk 数据。
		this->chunk_data = p;                                   // 保存新 chunk 数据。
		this->chunk_size = size;                                // 保存 chunk 大小。
		this->nreceived = size + 2;                              // 标记为完整数据。
		return true;                                            // 设置成功。
	}
	else
		return false;                                           // 分配失败。
}

int HttpMessageChunk::encode(struct iovec vectors[], int max)   // encode：编码 chunk。
{
	int len = sprintf(this->chunk_line, "%zx\r\n", this->chunk_size); // chunk 第一行是十六进制长度。

	vectors[0].iov_base = this->chunk_line;                      // iovec 0：chunk 长度行。
	vectors[0].iov_len = len;                                    // 长度行字节数。
	vectors[1].iov_base = this->chunk_data;                      // iovec 1：chunk 数据 + CRLF。
	vectors[1].iov_len = this->chunk_size + 2;                   // 数据长度加末尾 CRLF。

	return 2;                                                    // chunk 编码固定使用两个 iovec。
}

/*
  下面的 append_chunk_line() 和 append() 是 chunked transfer 解析逻辑。
  随机图库项目第一阶段通常直接设置 Content-Length 或返回普通 body，
  不一定会手写 chunked 响应，但面试中可以知道 Workflow 支持流式 chunk 解析。
*/

#define MIN(x, y)	((x) <= (y) ? (x) : (y))                    // MIN 宏：返回较小值。

int HttpMessageChunk::append_chunk_line(const void *buf, size_t size) // append_chunk_line：解析 chunk 长度行。
{
	char *end;                                                   // end：strtoul 解析结束位置。
	size_t i;                                                    // i：遍历游标。

	size = MIN(size, sizeof this->chunk_line - this->nreceived); // 防止写爆 chunk_line 缓冲区。
	memcpy(this->chunk_line + this->nreceived, buf, size);       // 把新收到的字节追加到 chunk_line。
	for (i = 0; i + 1 < this->nreceived + size; i++)             // 查找 CRLF。
	{
		if (this->chunk_line[i] == '\r')                         // 找到 CR。
		{
			if (this->chunk_line[i + 1] != '\n')                 // CR 后面必须是 LF。
			{
				errno = EBADMSG;                                // chunk 格式错误。
				return -1;                                      // 返回失败。
			}

			this->chunk_line[i] = '\0';                          // 把长度行截断成 C 字符串。
			this->chunk_size = strtoul(this->chunk_line, &end, 16); // 按十六进制解析 chunk 大小。
			if (end == this->chunk_line)                         // 没有解析出数字。
			{
				errno = EBADMSG;                                // 格式错误。
				return -1;                                      // 返回失败。
			}

			if (this->chunk_size > 64 * 1024 * 1024 ||           // 单个 chunk 超过 64MB。
				this->chunk_size > this->size_limit)             // 或超过消息大小限制。
			{
				errno = EMSGSIZE;                               // 消息过大。
				return -1;                                      // 返回失败。
			}

			this->chunk_data = malloc(this->chunk_size + 3);     // 分配 chunk 数据缓冲区。
			if (!this->chunk_data)                               // 分配失败。
				return -1;                                      // 返回失败。

			this->nreceived = i + 2;                             // 记录长度行已消费字节。
			return 1;                                           // 返回 1 表示长度行解析完成。
		}
	}

	if (i == sizeof this->chunk_line - 1)                        // 长度行太长仍未遇到 CRLF。
	{
		errno = EBADMSG;                                        // 认为格式错误。
		return -1;                                             // 返回失败。
	}

	this->nreceived += size;                                     // 继续等待更多长度行字节。
	return 0;                                                    // 返回 0 表示还没完成。
}

int HttpMessageChunk::append(const void *buf, size_t *size)     // append：解析 chunked 编码的一块数据。
{
	size_t nleft;                                                // nleft：当前 chunk 还需要接收的字节数。
	size_t n;                                                    // n：本次已处理字节数。
	int ret;                                                     // ret：解析状态。

	if (!this->chunk_data)                                       // 如果还没有 chunk_data，说明需要先解析长度行。
	{
		n = this->nreceived;                                     // 记录解析长度行前已接收字节数。
		ret = this->append_chunk_line(buf, *size);               // 尝试解析 chunk 长度行。
		if (ret <= 0)                                           // 未完成或出错。
			return ret;                                         // 直接返回。

		n = this->nreceived - n;                                 // n 变成长度行本次消费的字节数。
		this->nreceived = 0;                                     // 开始接收 chunk 数据，计数归零。
	}
	else
		n = 0;                                                   // 已有 chunk_data，直接接收数据。

	if (this->chunk_size != 0)                                   // 普通非结束 chunk。
	{
		nleft = this->chunk_size + 2 - this->nreceived;          // 还需要接收数据 + 末尾 CRLF 的字节数。
		if (*size - n > nleft)                                  // 如果输入数据超过当前 chunk 需要。
			*size = n + nleft;                                  // 调整 size，让上层知道只消费当前 chunk 字节。

		buf = (const char *)buf + n;                             // 跳过长度行已消费部分。
		n = *size - n;                                          // n 变成本次要复制的数据大小。
		memcpy((char *)this->chunk_data + this->nreceived, buf, n); // 复制 chunk 数据片段。
		this->nreceived += n;                                   // 更新已接收字节数。
		if (this->nreceived == this->chunk_size + 2)             // 数据和末尾 CRLF 已收完。
		{
			((char *)this->chunk_data)[this->nreceived] = '\0';  // 添加 '\0' 方便作为字符串查看。
			return 1;                                           // 返回 1 表示一个 chunk 完整。
		}
	}
	else                                                        // chunk_size == 0 表示最后一个 chunk。
	{
		while (n < *size)                                       // 消费 trailer 或最后 CRLF。
		{
			char c = ((const char *)buf)[n];                     // 当前字符。

			if (this->nreceived == 0)                            // 等待 CR。
			{
				if (c == '\r')                                  // 收到 CR。
					this->nreceived = 1;                         // 状态进入等待 LF。
				else
					this->nreceived = (size_t)-2;                // 可能是 trailer header。
			}
			else if (this->nreceived == 1)                       // 等待 LF。
			{
				if (c == '\n')                                  // 收到 LF，结束 chunked。
				{
					*size = n + 1;                              // 输出消费字节数。
					this->nreceived = 2;                         // 标记结束。
					((char *)this->chunk_data)[0] = '\r';        // 保存空 chunk 数据。
					((char *)this->chunk_data)[1] = '\n';
					((char *)this->chunk_data)[2] = '\0';
					return 1;                                   // 返回完整。
				}
				else
					break;                                      // 格式错误。
			}
			else if (this->nreceived == (size_t)-2)              // 正在读 trailer 行。
			{
				if (c == '\r')                                  // trailer 行遇到 CR。
					this->nreceived = (size_t)-1;                // 等待 LF。
			}
			else /* if (this->nreceived == (size_t)-1) */        // trailer CR 后等待 LF。
			{
				if (c == '\n')                                  // trailer 行结束。
					this->nreceived = 0;                         // 回到等待最终空行。
				else
					break;                                      // 格式错误。
			}

			n++;                                                // 继续消费下一个字符。
		}

		if (n < *size)                                          // 如果提前 break，说明格式不合法。
		{
			errno = EBADMSG;                                    // 设置坏消息错误。
			return -1;                                         // 返回失败。
		}
	}

	return 0;                                                    // 当前 chunk 尚未完整。
}

HttpMessageChunk::HttpMessageChunk(HttpMessageChunk&& msg) :   // chunk 移动构造函数。
	ProtocolMessage(std::move(msg))                             // 移动父类部分。
{
	memcpy(this->chunk_line, msg.chunk_line, sizeof this->chunk_line); // 复制 chunk 长度行缓冲区。
	this->chunk_data = msg.chunk_data;                           // 接管 chunk 数据指针。
	msg.chunk_data = NULL;                                       // 清空源对象指针。
	this->chunk_size = msg.chunk_size;                           // 接管 chunk 大小。
	this->nreceived = msg.nreceived;                             // 接管已接收字节数。
	msg.nreceived = 0;                                           // 源对象归零。
}

HttpMessageChunk& HttpMessageChunk::operator = (HttpMessageChunk&& msg) // chunk 移动赋值。
{
	if (&msg != this)                                           // 防止自我移动赋值。
	{
		*(ProtocolMessage *)this = std::move(msg);               // 移动父类部分。

		memcpy(this->chunk_line, msg.chunk_line, sizeof this->chunk_line); // 复制长度行缓冲。
		free(this->chunk_data);                                  // 释放当前已有 chunk 数据。
		this->chunk_data = msg.chunk_data;                       // 接管源对象数据。
		msg.chunk_data = NULL;                                   // 清空源对象指针。
		this->chunk_size = msg.chunk_size;                       // 接管 chunk 大小。
		this->nreceived = msg.nreceived;                         // 接管已接收字节数。
		msg.nreceived = 0;                                       // 源对象归零。
	}

	return *this;                                                // 返回当前对象引用。
}

}                                                              // namespace protocol

