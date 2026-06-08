/*
  注释版源码文件：RedisMessage.h

  原始文件位置：
  workflow-master/src/protocol/RedisMessage.h

  本文件用途：
  定义 Workflow 的 Redis 协议对象。

  对随机图库项目最重要：
  1. RedisRequest：用于设置 Redis 命令，比如 SRANDMEMBER / SADD / HGETALL。
  2. RedisResponse：用于读取 Redis 返回结果。
  3. RedisValue：用于承载 Redis 返回值，可能是字符串、整数、数组、nil、error。

  示例：
      WFRedisTask *task = WFTaskFactory::create_redis_task(url, 0, callback);
      task->get_req()->set_request("SRANDMEMBER", {"gallery:images", "5"});

  callback 中：
      RedisValue value;
      task->get_resp()->get_result(value);
*/

#ifndef _REDISMESSAGE_H_                                      // 头文件保护宏：防止重复 include。
#define _REDISMESSAGE_H_                                      // 定义头文件保护宏。

#include <stdint.h>                                           // 引入 int64_t 等固定宽度整数类型。
#include <string>                                             // 引入 std::string，用于 Redis 字符串值。
#include <vector>                                             // 引入 std::vector，用于 Redis 数组和命令参数。
#include "ProtocolMessage.h"                                  // 引入 ProtocolMessage，RedisMessage 要实现 encode/append。
#include "redis_parser.h"                                     // 引入底层 Redis parser 和 redis_reply_t。

namespace protocol                                            // protocol 命名空间：协议对象统一放这里。
{

class RedisValue                                              // RedisValue：C++ 风格的 Redis 返回值封装。
{
public:
	RedisValue();                                             // 构造函数：默认创建 nil 类型值。
	virtual ~RedisValue();                                    // 析构函数：释放内部 data_ 指向的数据。

	RedisValue(const RedisValue& copy);                       // 拷贝构造：深拷贝另一个 RedisValue。
	RedisValue& operator= (const RedisValue& copy);            // 拷贝赋值：释放旧数据并复制新数据。
	RedisValue(RedisValue&& move);                            // 移动构造：接管另一个 RedisValue 的内部数据。
	RedisValue& operator= (RedisValue&& move);                 // 移动赋值：释放旧数据并接管新数据。

	void set_nil();                                           // set_nil()：清空数据并把类型设置为 nil。
	void set_int(int64_t intv);                               // set_int(intv)：设置为整数类型。
	void set_string(const std::string& strv);                  // set_string(strv)：设置为普通字符串类型。
	void set_status(const std::string& strv);                  // set_status(strv)：设置为 Redis 状态回复，如 OK。
	void set_error(const std::string& strv);                   // set_error(strv)：设置为 Redis 错误回复。
	void set_string(const char *str, size_t len);              // set_string(str, len)：用字符数组设置字符串。
	void set_status(const char *str, size_t len);              // set_status(str, len)：用字符数组设置状态。
	void set_error(const char *str, size_t len);               // set_error(str, len)：用字符数组设置错误。
	void set_string(const char *str);                          // set_string(str)：用 C 字符串设置字符串。
	void set_status(const char *str);                          // set_status(str)：用 C 字符串设置状态。
	void set_error(const char *str);                           // set_error(str)：用 C 字符串设置错误。
	void set_array(size_t new_size);                           // set_array(new_size)：设置为数组类型，并调整数组大小。
	void set(const redis_reply_t *reply);                      // set(reply)：把 C 风格 redis_reply_t 转换成 RedisValue。

	bool is_ok() const;                                       // is_ok()：只要不是 error，就认为是 ok。
	bool is_error() const;                                    // is_error()：判断是否是 Redis 错误。
	bool is_nil() const;                                      // is_nil()：判断是否是 nil。
	bool is_int() const;                                      // is_int()：判断是否是整数。
	bool is_array() const;                                    // is_array()：判断是否是数组。
	bool is_string() const;                                   // is_string()：判断是否是 string 或 status。
	int get_type() const;                                     // get_type()：返回底层 Redis reply 类型。

	std::string string_value() const;                          // string_value()：复制返回字符串值；非字符串类型返回空串。
	const std::string *string_view() const;                    // string_view()：不复制，返回内部 string 指针；非字符串返回 NULL。
	int64_t int_value() const;                                 // int_value()：返回整数值；非整数返回 0。
	size_t arr_size() const;                                   // arr_size()：返回数组长度；非数组返回 0。
	void arr_clear();                                         // arr_clear()：清空数组；非数组不做事。
	void arr_resize(size_t new_size);                          // arr_resize(new_size)：调整数组大小；非数组不做事。
	RedisValue& arr_at(size_t pos) const;                      // arr_at(pos)：带越界检查地访问数组元素。
	RedisValue& operator[] (size_t pos) const;                 // operator[](pos)：不主动检查地访问数组元素。

	bool transform(redis_reply_t *reply) const;                // transform(reply)：把 RedisValue 转换回 C 风格 redis_reply_t。
	void clear();                                             // clear()：等价于 set_nil()。
	std::string debug_string() const;                          // debug_string()：生成调试文本。

private:
	void free_data();                                         // free_data()：释放 data_，根据 type_ 判断实际类型。
	void only_set_string_data(const std::string& strv);        // only_set_string_data(strv)：只设置 string 数据，不改 type_。
	void only_set_string_data(const char *str, size_t len);    // 字符数组版本。
	void only_set_string_data(const char *str);                // C 字符串版本。

	int type_;                                                // type_：Redis 返回类型，如 NIL/INTEGER/STRING/ARRAY/ERROR。
	void *data_;                                              // data_：实际数据指针，可能指向 int64_t、string、vector<RedisValue>。
};

class RedisMessage : public ProtocolMessage                    // RedisMessage：Redis 请求和响应的共同基类。
{
public:
	RedisMessage();                                           // 构造函数：初始化 Redis parser 和编码流。
	virtual ~RedisMessage();                                  // 析构函数：释放 parser 和 stream。
	RedisMessage(RedisMessage&& move);                        // 移动构造：接管 parser/stream 等资源。
	RedisMessage& operator= (RedisMessage&& move);             // 移动赋值。

public:
	bool parse_success() const;                               // parse_success()：判断 append 收到的数据是否解析成功。
	bool is_asking() const;                                   // is_asking()：Redis Cluster ASKING 状态相关。
	void set_asking(bool asking);                              // set_asking(asking)：设置 ASKING 标记。

protected:
	redis_parser_t *parser_;                                  // parser_：底层 Redis parser，负责 RESP 协议解析。

	virtual int encode(struct iovec vectors[], int max);        // encode(vectors, max)：把 Redis 消息编码成 iovec 用于发送。
	virtual int append(const void *buf, size_t *size);          // append(buf, size)：把收到的字节追加给 parser 解析。
	bool encode_reply(redis_reply_t *reply);                   // encode_reply(reply)：把 Redis reply 编码成 RESP 格式。

	class EncodeStream *stream_;                               // stream_：编码流，用来构造要发送的数据。

private:
	size_t cur_size_;                                          // cur_size_：当前编码/解析进度。
	bool asking_;                                             // asking_：Redis Cluster ASKING 标记。
};

class RedisRequest : public RedisMessage                       // RedisRequest：Redis 请求对象。
{
public:
	RedisRequest() = default;                                  // 默认构造。
	RedisRequest(RedisRequest&& move) = default;               // 默认移动构造。
	RedisRequest& operator= (RedisRequest&& move) = default;   // 默认移动赋值。

public:
	void set_request(const std::string& command,                // command：Redis 命令名，比如 "SRANDMEMBER"。
					 const std::vector<std::string>& params);  // params：命令参数数组，比如 {"gallery:images", "5"}。

	bool get_command(std::string& command) const;              // get_command(command)：服务端场景读取客户端发来的命令名。
	bool get_params(std::vector<std::string>& params) const;   // get_params(params)：服务端场景读取客户端命令参数。

protected:
	virtual int encode(struct iovec vectors[], int max);        // encode()：把 user_request_ 编码为 Redis RESP 请求。
	virtual int append(const void *buf, size_t *size);          // append()：服务端接收 Redis 请求时解析输入。

private:
	std::vector<std::string> user_request_;                    // user_request_：用户设置的命令和参数；第 0 个一般是 command。
};

class RedisResponse : public RedisMessage                      // RedisResponse：Redis 响应对象。
{
public:
	RedisResponse() = default;                                 // 默认构造。
	RedisResponse(RedisResponse&& move) = default;             // 默认移动构造。
	RedisResponse& operator= (RedisResponse&& move) = default; // 默认移动赋值。

public:
	void get_result(RedisValue& value) const;                  // get_result(value)：复制 Redis 返回结果到 RedisValue。

	bool set_result(const RedisValue& value);                  // set_result(value)：服务端场景设置要返回给客户端的 Redis 值。

public:
	redis_reply_t *result_ptr();                               // result_ptr()：直接返回底层 redis_reply_t 指针；不要手动 free。

protected:
	virtual int append(const void *buf, size_t *size);          // append()：客户端接收 Redis 响应时解析输入。

private:
	RedisValue value_;                                        // value_：服务端 set_result 时保存的 C++ 风格结果。
};

inline RedisValue::RedisValue():                              // RedisValue 默认构造。
	type_(REDIS_REPLY_TYPE_NIL),                               // type_ 初始化为 NIL。
	data_(NULL)                                                // data_ 初始化为空。
{
}

inline RedisValue::RedisValue(const RedisValue& copy):         // 拷贝构造。
	type_(REDIS_REPLY_TYPE_NIL),                               // 先初始化为空类型。
	data_(NULL)                                                // 先初始化空数据。
{
	this->operator= (copy);                                    // 复用拷贝赋值逻辑完成深拷贝。
}

inline RedisValue::RedisValue(RedisValue&& move):              // 移动构造。
	type_(REDIS_REPLY_TYPE_NIL),                               // 先初始化为空类型。
	data_(NULL)                                                // 先初始化空数据。
{
	this->operator= (std::move(move));                         // 复用移动赋值逻辑接管资源。
}

inline bool RedisValue::is_ok() const { return type_ != REDIS_REPLY_TYPE_ERROR; } // 非 error 都认为 ok。
inline bool RedisValue::is_error() const { return type_ == REDIS_REPLY_TYPE_ERROR; } // 是否 error。
inline bool RedisValue::is_nil() const { return type_ == REDIS_REPLY_TYPE_NIL; } // 是否 nil。
inline bool RedisValue::is_int() const { return type_ == REDIS_REPLY_TYPE_INTEGER; } // 是否整数。
inline bool RedisValue::is_array() const { return type_ == REDIS_REPLY_TYPE_ARRAY; } // 是否数组。
inline int RedisValue::get_type() const { return type_; }      // 返回 Redis 类型。

inline bool RedisValue::is_string() const                      // is_string()：判断是否可按字符串读取。
{
	return type_ == REDIS_REPLY_TYPE_STRING || type_ == REDIS_REPLY_TYPE_STATUS; // string 和 status 都有字符串数据。
}

inline std::string RedisValue::string_value() const            // string_value()：复制字符串结果。
{
	if (type_ == REDIS_REPLY_TYPE_STRING ||
		type_ == REDIS_REPLY_TYPE_STATUS ||
		type_ == REDIS_REPLY_TYPE_ERROR)                       // string/status/error 都用 std::string 保存。
		return *((std::string *)data_);                        // 强转并复制返回。
	else
		return "";                                            // 其他类型返回空字符串。
}

inline const std::string *RedisValue::string_view() const      // string_view()：返回内部 string 指针。
{
	if (type_ == REDIS_REPLY_TYPE_STRING ||
			type_ == REDIS_REPLY_TYPE_STATUS ||
			type_ == REDIS_REPLY_TYPE_ERROR)
		return ((std::string *)data_);                         // 返回内部地址，不复制。
	else
		return NULL;                                          // 其他类型没有 string。
}

inline int64_t RedisValue::int_value() const                   // int_value()：读取整数。
{
	if (type_ == REDIS_REPLY_TYPE_INTEGER)                     // 只有 INTEGER 类型才有 int64_t 数据。
		return *((int64_t *)data_);                            // 返回整数值。
	else
		return 0;                                             // 非整数返回 0。
}

inline size_t RedisValue::arr_size() const                     // arr_size()：读取数组大小。
{
	if (type_ == REDIS_REPLY_TYPE_ARRAY)                       // 只有 ARRAY 类型才有 vector 数据。
		return ((std::vector<RedisValue> *)data_)->size();     // 返回 vector 大小。
	else
		return 0;                                             // 非数组返回 0。
}

inline RedisValue& RedisValue::arr_at(size_t pos) const        // arr_at(pos)：带检查访问数组元素。
{
	return ((std::vector<RedisValue> *)data_)->at(pos);        // 使用 vector::at，越界会抛异常。
}

inline RedisValue& RedisValue::operator[] (size_t pos) const   // operator[](pos)：访问数组元素。
{
	return (*((std::vector<RedisValue> *)data_))[pos];         // 使用 vector[]，不主动检查越界。
}

inline void RedisValue::set_nil()                              // set_nil()：设置为空值。
{
	free_data();                                               // 先释放旧数据。
	type_ = REDIS_REPLY_TYPE_NIL;                              // 再把类型改为 NIL。
}

inline void RedisValue::clear()                                // clear()：清空当前值。
{
	set_nil();                                                 // 复用 set_nil。
}

inline bool RedisMessage::parse_success() const { return parser_->parse_succ; } // 是否解析成功。

inline bool RedisMessage::is_asking() const { return asking_; } // 返回 ASKING 标记。

inline void RedisMessage::set_asking(bool asking) { asking_ = asking; } // 设置 ASKING 标记。

inline redis_reply_t *RedisResponse::result_ptr()              // result_ptr()：获取底层回复结构。
{
	return &parser_->reply;                                    // 返回 parser 内部 reply 地址。
}

inline void RedisResponse::get_result(RedisValue& value) const // get_result(value)：获取 Redis 响应结果。
{
	if (parser_->parse_succ)                                   // 如果 RESP 解析成功。
		value.set(&parser_->reply);                            // 把底层 reply 转成 RedisValue。
	else                                                       // 如果解析失败。
		value.set_nil();                                       // 返回 nil，避免用户读到未定义数据。
}

}                                                             // namespace protocol 结束。

#endif                                                        // 结束头文件保护宏。
