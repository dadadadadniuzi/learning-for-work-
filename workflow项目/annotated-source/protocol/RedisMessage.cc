/*
  注释版源码文件：RedisMessage.cc

  原始文件位置：
  workflow-master/src/protocol/RedisMessage.cc

  本文件用途：
  实现 Redis RESP 协议消息的值对象、命令编码、响应解析。

  对随机图库项目最重要：
  - RedisRequest::set_request("SRANDMEMBER", {"gallery:images", "5"})
    会被编码成 Redis RESP 数组命令。
  - RedisResponse::get_result() 背后的 RedisValue 可以表示数组、字符串、整数、nil、error。
*/

#include <errno.h>                                             // errno、EMSGSIZE、EBADMSG。
#include <string.h>                                            // strlen、strcasecmp。
#include <sstream>                                             // std::ostringstream，用于 debug_string。
#include <utility>                                             // std::move。
#include "EncodeStream.h"                                      // EncodeStream：把协议内容写入 iovec。
#include "RedisMessage.h"                                      // RedisMessage/RedisRequest/RedisResponse。

namespace protocol                                             // Redis 协议相关类所在命名空间。
{

typedef int64_t Rint;                                          // Rint：Redis integer 类型在 C++ 中的存储类型。
typedef std::string Rstr;                                      // Rstr：Redis string/status/error 的存储类型。
typedef std::vector<RedisValue> Rarr;                          // Rarr：Redis array 的存储类型。

RedisValue& RedisValue::operator= (const RedisValue& copy)     // RedisValue 拷贝赋值。
{
	if (this != &copy)                                          // 防止自我赋值。
	{
		free_data();                                            // 先释放当前已有数据，避免内存泄漏。

		switch (copy.type_)                                     // 根据源对象类型深拷贝数据。
		{
		case REDIS_REPLY_TYPE_INTEGER:                          // 整数类型。
			type_ = copy.type_;                                 // 拷贝类型。
			data_ = new Rint(*((Rint*)(copy.data_)));            // 深拷贝整数值。
			break;

		case REDIS_REPLY_TYPE_ERROR:                            // error/status/string 都用 std::string 保存。
		case REDIS_REPLY_TYPE_STATUS:
		case REDIS_REPLY_TYPE_STRING:
			type_ = copy.type_;                                 // 拷贝类型。
			data_ = new Rstr(*((Rstr*)(copy.data_)));            // 深拷贝字符串。
			break;

		case REDIS_REPLY_TYPE_ARRAY:                            // 数组类型。
			type_ = copy.type_;                                 // 拷贝类型。
			data_ = new Rarr(*((Rarr*)(copy.data_)));            // 深拷贝数组，数组元素也会递归拷贝。
			break;

		default:                                                // nil 或未知类型。
			type_ = REDIS_REPLY_TYPE_NIL;                       // 设置为 nil。
			data_ = NULL;                                       // nil 不需要数据指针。
		}
	}

	return *this;                                               // 返回当前对象引用。
}

RedisValue& RedisValue::operator= (RedisValue&& move)          // RedisValue 移动赋值。
{
	if (this != &move)                                          // 防止自我移动。
	{
		free_data();                                            // 释放当前已有数据。

		type_ = move.type_;                                     // 接管源对象类型。
		data_ = move.data_;                                     // 接管源对象数据指针。

		move.type_ = REDIS_REPLY_TYPE_NIL;                      // 源对象改成 nil。
		move.data_ = NULL;                                      // 源对象指针清空，避免重复释放。
	}

	return *this;                                               // 返回当前对象引用。
}

void RedisValue::free_data()                                   // free_data()：按当前类型释放 data_。
{
	if (data_)                                                  // 只有 data_ 不为空才需要释放。
	{
		switch (type_)                                          // 根据类型选择正确 delete。
		{
		case REDIS_REPLY_TYPE_INTEGER:
			delete (Rint *)data_;                               // 删除整数对象。
			break;

		case REDIS_REPLY_TYPE_ERROR:
		case REDIS_REPLY_TYPE_STATUS:
		case REDIS_REPLY_TYPE_STRING:
			delete (Rstr *)data_;                               // 删除字符串对象。
			break;

		case REDIS_REPLY_TYPE_ARRAY:
			delete (Rarr *)data_;                               // 删除数组对象。
			break;
		}

		data_ = NULL;                                           // 释放后置空，避免悬空。
	}
}

void RedisValue::only_set_string_data(const std::string& strv) // only_set_string_data：当前 data_ 已经是 Rstr 时直接赋值。
{
	Rstr *p = (Rstr *)(data_);                                  // 把 void* 转回 std::string*。
	p->assign(strv);                                            // 用新字符串覆盖旧内容。
}

void RedisValue::only_set_string_data(const char *str, size_t len) // 设置指定长度字符串。
{
	Rstr *p = (Rstr *)(data_);                                  // 转成 std::string*。
	if (str == NULL || len == 0)                                // 空指针或长度 0。
		p->clear();                                             // 清空字符串。
	else
		p->assign(str, len);                                    // 拷贝 len 字节。
}

void RedisValue::set_int(int64_t intv)                          // set_int(intv)：把当前 RedisValue 设为整数。
{
	if (type_ == REDIS_REPLY_TYPE_INTEGER)                      // 如果本来就是整数。
		*((Rint *)data_) = intv;                                // 直接覆盖值，复用内存。
	else                                                        // 否则需要切换类型。
	{
		free_data();                                            // 释放旧数据。
		data_ = new Rint(intv);                                 // 创建整数数据。
		type_ = REDIS_REPLY_TYPE_INTEGER;                       // 设置类型为整数。
	}
}

void RedisValue::set_string(const std::string& strv)            // set_string(strv)：设为 bulk string。
{
	if (type_ == REDIS_REPLY_TYPE_STRING ||
		type_ == REDIS_REPLY_TYPE_STATUS ||
		type_ == REDIS_REPLY_TYPE_ERROR)                        // 如果当前本来就是字符串类存储。
		only_set_string_data(strv);                             // 复用 string 对象。
	else
	{
		free_data();                                            // 释放旧类型数据。
		data_ = new Rstr(strv);                                 // 新建字符串。
	}

	type_ = REDIS_REPLY_TYPE_STRING;                            // 类型标记为 string。
}

void RedisValue::set_status(const std::string& strv)            // set_status(strv)：设为 Redis 状态回复，例如 OK。
{
	if (type_ == REDIS_REPLY_TYPE_STRING ||
		type_ == REDIS_REPLY_TYPE_STATUS ||
		type_ == REDIS_REPLY_TYPE_ERROR)
		only_set_string_data(strv);                             // 复用字符串存储。
	else
	{
		free_data();                                            // 释放旧数据。
		data_ = new Rstr(strv);                                 // 新建字符串。
	}

	type_ = REDIS_REPLY_TYPE_STATUS;                            // 类型标记为 status。
}

void RedisValue::set_error(const std::string& strv)             // set_error(strv)：设为 Redis 错误回复。
{
	if (type_ == REDIS_REPLY_TYPE_STRING ||
		type_ == REDIS_REPLY_TYPE_STATUS ||
		type_ == REDIS_REPLY_TYPE_ERROR)
		only_set_string_data(strv);                             // 复用字符串存储。
	else
	{
		free_data();                                            // 释放旧数据。
		data_ = new Rstr(strv);                                 // 新建字符串。
	}

	type_ = REDIS_REPLY_TYPE_ERROR;                             // 类型标记为 error。
}

void RedisValue::set_string(const char *str, size_t len)        // set_string(str,len)：设为指定长度 string。
{
	if (type_ == REDIS_REPLY_TYPE_STRING ||
		type_ == REDIS_REPLY_TYPE_STATUS ||
		type_ == REDIS_REPLY_TYPE_ERROR)
		only_set_string_data(str, len);                         // 复用 string 对象。
	else
	{
		free_data();                                            // 释放旧数据。
		data_ = new Rstr(str, len);                             // 新建 string。
	}

	type_ = REDIS_REPLY_TYPE_STRING;                            // 标记为 string。
}

void RedisValue::set_array(size_t new_size)                     // set_array(new_size)：设为数组，并调整数组大小。
{
	if (type_ == REDIS_REPLY_TYPE_ARRAY)                        // 如果本来就是数组。
		((Rarr *)data_)->resize(new_size);                      // 直接 resize。
	else                                                        // 否则切换为数组。
	{
		free_data();                                            // 释放旧数据。
		data_ = new Rarr(new_size);                             // 创建 RedisValue 数组。
		type_ = REDIS_REPLY_TYPE_ARRAY;                         // 类型标记为 array。
	}
}

void RedisValue::set(const redis_reply_t *reply)                // set(reply)：把底层 C parser 的 redis_reply_t 转成 C++ RedisValue。
{
	set_nil();                                                  // 先清空成 nil，避免旧数据残留。
	switch (reply->type)                                        // 根据底层 reply 类型转换。
	{
	case REDIS_REPLY_TYPE_INTEGER:
		set_int(reply->integer);                                // 整数回复。
		break;

	case REDIS_REPLY_TYPE_ERROR:
		set_error(reply->str, reply->len);                      // 错误回复。
		break;

	case REDIS_REPLY_TYPE_STATUS:
		set_status(reply->str, reply->len);                     // 状态回复。
		break;

	case REDIS_REPLY_TYPE_STRING:
		set_string(reply->str, reply->len);                     // bulk string 回复。
		break;

	case REDIS_REPLY_TYPE_ARRAY:
		set_array(reply->elements);                             // 创建同样大小的数组。

		if (reply->elements > 0)                                // 如果数组非空。
		{
			Rarr *parr = (Rarr *)data_;                         // 取出 C++ 数组。
			for (size_t i = 0; i < reply->elements; i++)         // 遍历每个元素。
				(*parr)[i].set(reply->element[i]);              // 递归转换子元素。
		}

		break;
	}
}

bool RedisValue::transform(redis_reply_t *reply) const          // transform(reply)：把 C++ RedisValue 转回底层 redis_reply_t。
{
	//todo risk of stack overflow                               // 深层数组递归可能栈溢出，这是源码作者留下的提示。
	Rarr *parr;                                                  // parr：数组指针。
	Rstr *pstr;                                                  // pstr：字符串指针。

	redis_reply_set_null(reply);                                // 先把目标 reply 清空成 null。
	switch (type_)                                              // 按当前类型编码。
	{
	case REDIS_REPLY_TYPE_INTEGER:
		redis_reply_set_integer(*((Rint *)data_), reply);        // 设置整数。
		break;

	case REDIS_REPLY_TYPE_ARRAY:
		parr = (Rarr *)data_;                                   // 取出数组。
		if (redis_reply_set_array(parr->size(), reply) < 0)      // 为底层 reply 分配数组。
			return false;                                       // 分配失败。

		for (size_t i = 0; i < reply->elements; i++)             // 遍历元素。
		{
			if (!(*parr)[i].transform(reply->element[i]))        // 递归转换。
				return false;                                   // 子元素失败则整体失败。
		}

		break;

	case REDIS_REPLY_TYPE_STATUS:
		pstr = (Rstr *)data_;                                   // 取出状态字符串。
		redis_reply_set_status(pstr->c_str(), pstr->size(), reply); // 设置 status。
		break;

	case REDIS_REPLY_TYPE_ERROR:
		pstr = (Rstr *)data_;                                   // 取出错误字符串。
		redis_reply_set_error(pstr->c_str(), pstr->size(), reply); // 设置 error。
		break;

	case REDIS_REPLY_TYPE_STRING:
		pstr = (Rstr *)data_;                                   // 取出 bulk string。
		redis_reply_set_string(pstr->c_str(), pstr->size(), reply); // 设置 string。
		break;
	}

	return true;                                                // 转换成功。
}

std::string RedisValue::debug_string() const                   // debug_string()：生成便于打印的调试字符串。
{
	std::string ret;                                            // ret：最终输出字符串。

	if (is_error())                                             // 错误类型。
	{
		ret += "ERROR: ";                                       // 加错误前缀。
		ret += string_view()->c_str();                          // 加错误内容。
	}
	else if (is_int())                                          // 整数类型。
	{
		std::ostringstream oss;                                 // 用流转字符串。
		oss << int_value();                                     // 写入整数值。
		ret += oss.str();                                       // 拼接结果。
	}
	else if (is_nil())                                          // nil 类型。
	{
		ret += "nil";                                           // 输出 nil。
	}
	else if (is_string())                                       // 字符串类型。
	{
		ret += '\"';                                            // 左引号。
		ret += string_view()->c_str();                          // 内容。
		ret += '\"';                                            // 右引号。
	}
	else if (is_array())                                        // 数组类型。
	{
		ret += '[';                                             // 数组左括号。
		size_t l = arr_size();                                  // 数组长度。
		for (size_t i = 0; i < l; i++)                          // 遍历元素。
		{
			if (i)                                              // 非第一个元素。
				ret += ", ";                                    // 加分隔符。

			ret += (*this)[i].debug_string();                   // 递归生成子元素字符串。
		}
		ret += ']';                                             // 数组右括号。
	}

	return ret;                                                 // 返回调试字符串。
}

RedisValue::~RedisValue()                                      // 析构函数。
{
	free_data();                                                // 释放当前保存的数据。
}

RedisMessage::RedisMessage():                                  // RedisMessage 构造函数。
	parser_(new redis_parser_t),                                // 创建 Redis parser。
	stream_(new EncodeStream),                                  // 创建编码流。
	cur_size_(0),                                               // 当前接收大小为 0。
	asking_(false)                                             // Redis Cluster ASKING 状态初始为 false。
{
	redis_parser_init(parser_);                                 // 初始化 Redis parser。
}

RedisMessage::~RedisMessage()                                  // RedisMessage 析构函数。
{
	if (parser_)                                                // 如果 parser_ 不为空。
	{
		redis_parser_deinit(parser_);                           // 反初始化 parser。
		delete parser_;                                         // 释放 parser 对象。
		delete stream_;                                         // 释放编码流。
	}
}

RedisMessage::RedisMessage(RedisMessage&& move) :              // RedisMessage 移动构造。
	ProtocolMessage(std::move(move))                            // 移动父类。
{
	parser_ = move.parser_;                                     // 接管 parser。
	stream_ = move.stream_;                                     // 接管 stream。
	cur_size_ = move.cur_size_;                                 // 接管当前大小。
	asking_ = move.asking_;                                     // 接管 ASKING 状态。

	move.parser_ = NULL;                                        // 清空源对象 parser。
	move.stream_ = NULL;                                        // 清空源对象 stream。
	move.cur_size_ = 0;                                         // 源对象大小归零。
	move.asking_ = false;                                       // 源对象 ASKING 状态归零。
}

bool RedisMessage::encode_reply(redis_reply_t *reply)           // encode_reply(reply)：把 redis_reply_t 编码成 RESP 文本协议。
{
	EncodeStream& stream = *stream_;                            // stream：输出编码流。
	switch (reply->type)                                        // 按 Redis reply 类型编码。
	{
	case REDIS_REPLY_TYPE_STATUS:
		stream << "+" << std::make_pair(reply->str, reply->len) << "\r\n"; // +OK\r\n 这种状态回复。
		break;

	case REDIS_REPLY_TYPE_ERROR:
		stream << "-" << std::make_pair(reply->str, reply->len) << "\r\n"; // -ERR ...\r\n 错误回复。
		break;

	case REDIS_REPLY_TYPE_NIL:
		stream << "$-1\r\n";                                  // Redis nil bulk string。
		break;

	case REDIS_REPLY_TYPE_INTEGER:
		stream << ":" << reply->integer << "\r\n";             // :123\r\n 整数回复。
		break;

	case REDIS_REPLY_TYPE_STRING:
		stream << "$" << reply->len << "\r\n";                 // bulk string 长度行。
		stream << std::make_pair(reply->str, reply->len) << "\r\n"; // bulk string 内容和 CRLF。
		break;

	case REDIS_REPLY_TYPE_ARRAY:
		stream << "*" << reply->elements << "\r\n";            // 数组长度行，例如 *3\r\n。
		for (size_t i = 0; i < reply->elements; i++)            // 遍历数组元素。
			if (!encode_reply(reply->element[i]))               // 递归编码每个元素。
				return false;                                  // 任一失败则整体失败。

		break;

	default:
		return false;                                          // 未知类型，编码失败。
	}

	return true;                                               // 编码成功。
}

int RedisMessage::encode(struct iovec vectors[], int max)       // encode(vectors,max)：编码 Redis 消息到 iovec。
{
	stream_->reset(vectors, max);                               // 让 EncodeStream 使用调用者提供的 iovec 数组。

	if (encode_reply(&parser_->reply))                          // 把 parser_ 中的 reply 结构编码进去。
		return stream_->size();                                 // 返回使用了多少个 iovec。

	return 0;                                                   // 编码失败或空消息。
}

int RedisMessage::append(const void *buf, size_t *size)         // append(buf,size*)：解析收到的 Redis RESP 字节。
{
	int ret = redis_parser_append_message(buf, size, parser_);   // 调用底层 Redis parser，size 输出实际消费字节数。

	if (ret >= 0)                                               // 解析没有协议错误。
	{
		cur_size_ += *size;                                     // 累加已接收字节。
		if (cur_size_ > this->size_limit)                       // 超过消息大小限制。
		{
			errno = EMSGSIZE;                                   // 设置消息过大。
			ret = -1;                                          // 返回错误。
		}
	}
	else if (ret == -2)                                         // parser 表示 RESP 格式错误。
	{
		errno = EBADMSG;                                       // 转成标准坏消息错误。
		ret = -1;                                              // 对外统一返回 -1。
	}

	return ret;                                                 // 0 未完整，>0 完整，-1 错误。
}

void RedisRequest::set_request(const std::string& command,      // command：Redis 命令名，例如 SRANDMEMBER。
							   const std::vector<std::string>& params) // params：命令参数列表。
{
	size_t n = params.size() + 1;                               // n：数组元素总数 = 命令名 + 参数个数。
	user_request_.reserve(n);                                   // 预留空间，减少 vector 扩容。
	user_request_.clear();                                      // 清空旧请求。
	user_request_.push_back(command);                           // 第一个元素放命令名。
	for (size_t i = 0; i < params.size(); i++)                   // 遍历参数。
		user_request_.push_back(params[i]);                     // 依次加入请求数组。

	redis_reply_t *reply = &parser_->reply;                     // reply：底层 Redis reply 结构，这里复用它表示请求数组。
	redis_reply_set_array(n, reply);                            // 设置为 RESP 数组，长度为 n。
	for (size_t i = 0; i < n; i++)                               // 遍历命令和参数。
	{
		redis_reply_set_string(user_request_[i].c_str(),         // 设置第 i 个数组元素为 bulk string。
							   user_request_[i].size(),
							   reply->element[i]);
	}
}

bool RedisRequest::get_command(std::string& command) const      // get_command(command)：从请求中取命令名。
{
	const redis_reply_t *reply = &parser_->reply;                // reply：底层请求数组。
	if (reply->type == REDIS_REPLY_TYPE_ARRAY && reply->elements > 0) // 必须是非空数组。
	{
		reply = reply->element[0];                              // 第一个元素是命令名。
		if (reply->type == REDIS_REPLY_TYPE_STRING)             // 命令必须是字符串。
		{
			command.assign(reply->str, reply->len);             // 输出命令字符串。
			return true;                                       // 获取成功。
		}
	}

	return false;                                              // 格式不符合，获取失败。
}

bool RedisRequest::get_params(std::vector<std::string>& params) const // get_params(params)：取命令参数。
{
	const redis_reply_t *reply = &parser_->reply;                // reply：底层请求数组。
	if (reply->type == REDIS_REPLY_TYPE_ARRAY && reply->elements > 0) // 必须是非空数组。
	{
		for (size_t i = 1; i < reply->elements; i++)             // 从第 1 个元素开始检查参数。
		{
			if (reply->element[i]->type != REDIS_REPLY_TYPE_STRING &&
				reply->element[i]->type != REDIS_REPLY_TYPE_NIL) // 参数只允许 string 或 nil。
			{
				return false;                                  // 有非法参数类型。
			}
		}

		params.reserve(reply->elements - 1);                    // 预留参数数量。
		params.clear();                                         // 清空旧参数。
		for (size_t i = 1; i < reply->elements; i++)             // 遍历参数元素。
			params.emplace_back(reply->element[i]->str, reply->element[i]->len); // 拷贝参数字符串。

		return true;                                           // 获取成功。
	}

	return false;                                              // 请求不是合法数组。
}

#define REDIS_ASK_COMMAND	"ASKING"                            // Redis Cluster ASKING 命令。
#define REDIS_ASK_REQUEST	"*1\r\n$6\r\nASKING\r\n"            // ASKING 命令的 RESP 编码。
#define REDIS_OK_RESPONSE	"+OK\r\n"                           // OK 响应。

int RedisRequest::encode(struct iovec vectors[], int max)       // RedisRequest::encode：编码 Redis 请求。
{
	stream_->reset(vectors, max);                               // 重置编码流。

	if (is_asking())                                            // 如果处于 ASKING 状态。
		(*stream_) << REDIS_ASK_REQUEST;                        // 先发送 ASKING 命令。
	if (encode_reply(&parser_->reply))                          // 再编码真正请求。
		return stream_->size();                                 // 返回 iovec 数量。

	return 0;                                                   // 编码失败。
}

int RedisRequest::append(const void *buf, size_t *size)         // RedisRequest::append：服务端场景解析 Redis 请求。
{
	int ret = RedisMessage::append(buf, size);                  // 先按普通 Redis 消息解析。

	if (ret > 0)                                                // 如果解析到完整请求。
	{
		std::string command;                                    // command：命令名。

		if (get_command(command) &&                              // 成功取到命令名。
			strcasecmp(command.c_str(), REDIS_ASK_COMMAND) == 0) // 命令是 ASKING。
		{
			redis_parser_deinit(parser_);                       // 清理当前 parser。
			redis_parser_init(parser_);                         // 重置 parser，等待后续真正请求。
			set_asking(true);                                   // 标记 ASKING 状态。

			ret = this->feedback(REDIS_OK_RESPONSE, strlen(REDIS_OK_RESPONSE)); // 反馈 +OK。
			if (ret != strlen(REDIS_OK_RESPONSE))               // 如果反馈不完整。
			{
				errno = ENOBUFS;                                // 设置缓冲不足。
				ret = -1;                                      // 返回错误。
			}
			else
				ret = 0;                                       // ASKING 不是最终请求，继续等下一条。
		}
	}

	return ret;                                                 // 返回解析状态。
}

int RedisResponse::append(const void *buf, size_t *size)        // RedisResponse::append：客户端场景解析 Redis 响应。
{
	int ret = RedisMessage::append(buf, size);                  // 先解析普通响应。

	if (ret > 0 && is_asking())                                 // 如果 ASKING 的 OK 响应已完成。
	{
		redis_parser_deinit(parser_);                           // 清理 parser。
		redis_parser_init(parser_);                             // 重置 parser，继续等真正业务响应。
		ret = 0;                                                // 当前响应不算最终完成。
		set_asking(false);                                      // 清除 ASKING 状态。
	}

	return ret;                                                 // 返回解析状态。
}

bool RedisResponse::set_result(const RedisValue& value)         // set_result(value)：手动设置 Redis 响应结果。
{
	redis_reply_t *reply = &parser_->reply;                     // reply：底层响应结构。
	redis_reply_deinit(reply);                                  // 清理旧 reply。
	redis_reply_init(reply);                                    // 初始化新 reply。

	value_ = value;                                             // 保存 C++ RedisValue。
	return value_.transform(reply);                             // 同步转换到底层 reply，供 encode 使用。
}

}                                                              // namespace protocol

