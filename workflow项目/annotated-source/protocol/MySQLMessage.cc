/*
  注释版源码文件：MySQLMessage.cc

  原始文件位置：
  workflow-master/src/protocol/MySQLMessage.cc

  本文件用途：
  实现 MySQL 协议消息的封包、拆包、认证、查询请求和响应解析。

  对随机图库项目最重要：
  1. MySQLRequest::set_query() 如何把 SQL 放进 MySQL COM_QUERY 包。
  2. MySQLMessage::encode() 如何把业务 payload 切成 MySQL packet。
  3. MySQLMessage::append() 如何把 socket 收到的字节拼成完整 MySQL packet 后解析。
  4. MySQLResponse 如何判断 OK 包、ERROR 包，以及读取 affected rows、last insert id。
*/

#include <stdint.h>                                            // uint8_t、uint16_t、uint32_t。
#include <string.h>                                            // memcpy、strlen。
#include <errno.h>                                             // errno、EMSGSIZE、EBADMSG、EOVERFLOW、EINVAL。
#include <string>                                              // std::string。
#include <openssl/sha.h>                                       // SHA1、SHA256，用于 MySQL 密码认证。
#include <openssl/rsa.h>                                       // RSA 相关接口。
#include <openssl/pem.h>                                       // PEM_read_bio_PUBKEY。
#include <openssl/evp.h>                                       // EVP_PKEY 等 OpenSSL 高层接口。
#include <utility>                                             // std::move。
#include "SSLWrapper.h"                                       // SSL 握手包装。
#include "mysql_byteorder.h"                                  // int3store、uint4korr 等 MySQL 字节序工具。
#include "mysql_types.h"                                      // MySQL 包类型、命令类型定义。
#include "MySQLResult.h"                                      // MySQLResultCursor，用于读取结果集。
#include "MySQLMessage.h"                                     // MySQLMessage/MySQLRequest/MySQLResponse 定义。

namespace protocol                                             // MySQL 协议代码所在命名空间。
{

#define MYSQL_PAYLOAD_MAX	((1 << 24) - 1)                    // MySQL 单个 packet payload 最大长度：16MB - 1。

#define MYSQL_NATIVE_PASSWORD	"mysql_native_password"        // MySQL 旧认证插件名。
#define CACHING_SHA2_PASSWORD	"caching_sha2_password"        // MySQL 8 默认认证插件名。
#define MYSQL_CLEAR_PASSWORD	"mysql_clear_password"         // 明文密码插件名。

MySQLMessage::~MySQLMessage()                                  // MySQLMessage 析构函数。
{
	if (parser_)                                                // 如果 parser_ 存在。
	{
		mysql_parser_deinit(parser_);                           // 反初始化 MySQL parser。
		mysql_stream_deinit(stream_);                           // 反初始化 MySQL stream。
		delete parser_;                                         // 释放 parser 对象。
		delete stream_;                                         // 释放 stream 对象。
	}
}

MySQLMessage::MySQLMessage(MySQLMessage&& move) :              // MySQLMessage 移动构造函数。
	ProtocolMessage(std::move(move))                            // 先移动父类 ProtocolMessage。
{
	parser_ = move.parser_;                                     // 接管 parser。
	stream_ = move.stream_;                                     // 接管 stream。
	seqid_ = move.seqid_;                                      // 接管当前 packet 序号。
	cur_size_ = move.cur_size_;                                 // 接管当前已接收大小。

	move.parser_ = NULL;                                        // 清空源对象 parser。
	move.stream_ = NULL;                                        // 清空源对象 stream。
	move.seqid_ = 0;                                            // 源对象 seqid 归零。
	move.cur_size_ = 0;                                         // 源对象大小归零。
}

MySQLMessage& MySQLMessage::operator= (MySQLMessage&& move)     // MySQLMessage 移动赋值。
{
	if (this != &move)                                          // 防止自我移动赋值。
	{
		*(ProtocolMessage *)this = std::move(move);              // 移动父类部分。

		if (parser_)                                            // 如果当前对象已有 parser。
		{
			mysql_parser_deinit(parser_);                       // 清理旧 parser。
			mysql_stream_deinit(stream_);                       // 清理旧 stream。
			delete parser_;                                     // 释放旧 parser。
			delete stream_;                                     // 释放旧 stream。
		}

		parser_ = move.parser_;                                 // 接管源 parser。
		stream_ = move.stream_;                                 // 接管源 stream。
		seqid_ = move.seqid_;                                  // 接管序号。
		cur_size_ = move.cur_size_;                             // 接管大小。

		move.parser_ = NULL;                                    // 清空源 parser。
		move.stream_ = NULL;                                    // 清空源 stream。
		move.seqid_ = 0;                                        // 源序号归零。
		move.cur_size_ = 0;                                     // 源大小归零。
	}

	return *this;                                               // 返回当前对象引用。
}

int MySQLMessage::append(const void *buf, size_t *size)         // append(buf,size*)：解析收到的 MySQL 字节流。
{
	const void *stream_buf;                                     // stream_buf：mysql_stream 拼出的完整 packet payload。
	size_t stream_len;                                          // stream_len：完整 payload 长度。
	size_t nleft = *size;                                       // nleft：本次输入还剩多少字节未处理。
	size_t n;                                                   // n：本轮写入 stream 的字节数。
	int ret;                                                    // ret：解析状态。

	cur_size_ += *size;                                         // 累加已接收字节数。
	if (cur_size_ > this->size_limit)                           // 如果超过消息大小限制。
	{
		errno = EMSGSIZE;                                       // 设置消息过大。
		return -1;                                             // 返回失败。
	}

	do                                                          // 可能一次收到多个 packet，所以循环处理。
	{
		n = nleft;                                              // 尝试消费剩余全部字节。
		ret = mysql_stream_write(buf, &n, stream_);              // 写入 MySQL stream；n 输出实际消费字节。
		if (ret > 0)                                            // ret > 0 表示拼出一个完整 MySQL packet。
		{
			seqid_ = mysql_stream_get_seq(stream_);              // 更新当前 packet 序号。
			mysql_stream_get_buf(&stream_buf, &stream_len, stream_); // 获取完整 payload。
			ret = decode_packet((const unsigned char *)stream_buf, stream_len); // 交给具体消息类型解析。
			if (ret == -2)                                      // decode_packet 用 -2 表示协议不完整或坏包。
				errno = EBADMSG;                                // 转为坏消息错误。
		}

		if (ret < 0)                                            // stream 或 decode 出错。
			return -1;                                          // 返回失败。

		buf = (const char *)buf + n;                             // 输入指针后移 n 字节。
		nleft -= n;                                             // 剩余字节减少。
	} while (nleft > 0);                                        // 直到本次输入全部处理完。

	return ret;                                                 // 返回最后一次解析状态。
}

int MySQLMessage::encode(struct iovec vectors[], int max)       // encode(vectors,max)：把 buf_ 编码成 MySQL packet iovec。
{
	const unsigned char *p = (unsigned char *)buf_.c_str();      // p：业务 payload 起始位置。
	size_t nleft = buf_.size();                                 // nleft：剩余 payload 字节数。
	uint8_t seqid_start = seqid_;                               // seqid_start：起始序号，用于检测绕回。
	uint8_t seqid = seqid_;                                     // seqid：当前 packet 序号。
	unsigned char *head;                                        // head：当前 packet 4 字节头。
	uint32_t length;                                            // length：当前 packet payload 长度。
	int i = 0;                                                  // i：当前使用的 iovec 数量。

	do                                                          // 大 payload 需要拆成多个 MySQL packet。
	{
		length = (nleft >= MYSQL_PAYLOAD_MAX ? MYSQL_PAYLOAD_MAX // 如果剩余超过单包上限，当前包取最大值。
											 : (uint32_t)nleft); // 否则取剩余大小。
		head = heads_[seqid];                                   // 每个 seqid 对应一块 packet header 缓冲。
		int3store(head, length);                                // MySQL header 前 3 字节保存 payload 长度。
		head[3] = seqid++;                                      // 第 4 字节保存 sequence id，然后自增。
		vectors[i].iov_base = head;                             // iovec 写入 packet header。
		vectors[i].iov_len = 4;                                 // header 固定 4 字节。
		i++;                                                    // iovec 数量加一。
		vectors[i].iov_base = const_cast<unsigned char *>(p);    // iovec 写入 payload 指针。
		vectors[i].iov_len = length;                            // payload 长度。
		i++;                                                    // iovec 数量加一。

		if (i > max)                                            // 如果超过调用者提供的 iovec 容量。
			break;                                              // 跳出并报 overflow。

		if (nleft < MYSQL_PAYLOAD_MAX)                          // 如果已经是最后一包。
			return i;                                           // 返回使用的 iovec 数量。

		nleft -= MYSQL_PAYLOAD_MAX;                             // 减掉已发送最大 payload。
		p += length;                                            // payload 指针后移。
	} while (seqid != seqid_start);                             // 防止 seqid 绕回导致无限循环。

	errno = EOVERFLOW;                                          // iovec 不够或 seqid 绕回。
	return -1;                                                  // 编码失败。
}

void MySQLRequest::set_query(const char *query, size_t length)  // set_query(query,length)：设置 SQL 查询请求。
{
	set_command(MYSQL_COM_QUERY);                               // 设置 MySQL 命令类型为 COM_QUERY。
	buf_.resize(length + 1);                                    // payload = 1 字节命令 + SQL 文本。
	char *buffer = const_cast<char *>(buf_.c_str());             // 取出可写 buffer。

	buffer[0] = MYSQL_COM_QUERY;                                // 第 1 字节写命令码。
	if (length > 0)                                             // 如果 SQL 非空。
		memcpy(buffer + 1, query, length);                      // 把 SQL 文本复制到命令码后面。
}

std::string MySQLRequest::get_query() const                    // get_query()：从请求对象中取 SQL 文本。
{
	size_t len = buf_.size();                                   // len：payload 长度。
	if (len <= 1 || buf_[0] != MYSQL_COM_QUERY)                 // 如果没有 SQL 或命令不是 COM_QUERY。
		return "";                                             // 返回空字符串。

	return std::string(buf_.c_str() + 1);                       // 跳过命令字节，返回 SQL 文本。
}

/*
  下面这些 capability flags 是 MySQL 握手认证阶段使用的能力位。
  随机图库项目中你通常只会写 create_mysql_task + set_query，
  但面试深入时可以知道 Workflow 内部也处理了 MySQL 登录和 SSL 升级。
*/

#define MYSQL_CAPFLAG_CLIENT_SSL				0x00000800      // 客户端支持 SSL。
#define MYSQL_CAPFLAG_CLIENT_PROTOCOL_41		0x00000200      // 使用 4.1+ 协议。
#define MYSQL_CAPFLAG_CLIENT_SECURE_CONNECTION	0x00008000      // 支持安全认证。
#define MYSQL_CAPFLAG_CLIENT_CONNECT_WITH_DB	0x00000008      // 登录时指定数据库。
#define MYSQL_CAPFLAG_CLIENT_MULTI_STATEMENTS	0x00010000      // 支持多语句。
#define MYSQL_CAPFLAG_CLIENT_MULTI_RESULTS		0x00020000      // 支持多结果集。
#define MYSQL_CAPFLAG_CLIENT_PS_MULTI_RESULTS	0x00040000      // 支持预处理语句多结果。
#define MYSQL_CAPFLAG_CLIENT_PLUGIN_AUTH		0x00080000      // 支持插件认证。
#define MYSQL_CAPFLAG_CLIENT_LOCAL_FILES		0x00000080      // 支持 LOAD DATA LOCAL。

int MySQLHandshakeResponse::encode(struct iovec vectors[], int max) // encode：服务端握手包编码。
{
	const char empty[10] = {0};                                 // empty：保留字段填 0。
	uint16_t cap_flags_lower = capability_flags_ & 0xffffffff;  // cap_flags_lower：能力位低 16 位。
	uint16_t cap_flags_upper = capability_flags_ >> 16;         // cap_flags_upper：能力位高 16 位。

	buf_.clear();                                               // 清空 payload。
	buf_.append((const char *)&protocol_version_, 1);            // 写协议版本。
	buf_.append(server_version_.c_str(), server_version_.size() + 1); // 写 server version，以 \0 结尾。
	buf_.append((const char *)&connection_id_, 4);               // 写连接 id。
	buf_.append((const char *)auth_plugin_data_, 8);             // 写认证随机种子前 8 字节。
	buf_.append(empty, 1);                                      // 写 1 字节 filler。
	buf_.append((const char *)&cap_flags_lower, 2);              // 写能力位低 16 位。
	buf_.append((const char *)&character_set_, 1);               // 写字符集。
	buf_.append((const char *)&status_flags_, 2);                // 写状态位。
	buf_.append((const char *)&cap_flags_upper, 2);              // 写能力位高 16 位。
	buf_.push_back(21);                                         // 写 auth plugin data 长度。
	buf_.append(empty, 10);                                     // 写保留字段。
	buf_.append((const char *)auth_plugin_data_ + 8, 12);        // 写认证随机种子剩余部分。
	buf_.push_back(0);                                          // 以 \0 结束。
	if (capability_flags_ & MYSQL_CAPFLAG_CLIENT_PLUGIN_AUTH)    // 如果支持插件认证。
		buf_.append(MYSQL_NATIVE_PASSWORD, strlen(MYSQL_NATIVE_PASSWORD) + 1); // 写认证插件名。

	return MySQLMessage::encode(vectors, max);                  // 交给通用 packet 编码。
}

int MySQLHandshakeResponse::decode_packet(const unsigned char *buf, size_t buflen) // decode_packet：解析服务端握手包。
{
	const unsigned char *end = buf + buflen;                    // end：输入末尾。
	const unsigned char *pos;                                   // pos：遍历游标。
	uint16_t cap_flags_lower;                                   // cap_flags_lower：能力低位。
	uint16_t cap_flags_upper;                                   // cap_flags_upper：能力高位。

	if (buflen == 0)                                            // 空包不完整。
		return -2;                                             // 返回 -2。

	protocol_version_ = *buf;                                  // 第一个字节是协议版本。
	if (protocol_version_ == 255)                               // 255 表示 error packet。
	{
		if (buflen >= 4)                                       // 至少有错误包头。
		{
			const_cast<unsigned char *>(buf)[3] = '#';           // 调整格式让 mysql_parser_parse 能识别 SQL state。
			if (mysql_parser_parse(buf, buflen, parser_) == 1)   // 尝试解析错误包。
			{
				disallowed_ = true;                             // 标记连接被拒绝。
				return 1;                                      // 错误包本身解析完成。
			}
		}

		errno = EBADMSG;                                       // 错误包格式也不对。
		return -1;                                             // 返回失败。
	}

	pos = ++buf;                                                // 跳过协议版本，开始读 server_version。
	while (pos < end && *pos)                                   // 查找 server_version 的 \0。
		pos++;                                                  // 游标后移。

	if (pos >= end || end - pos < 45)                           // 握手包剩余长度不足。
		return -2;                                             // 表示不完整。

	server_version_.assign((const char *)buf, pos - buf);        // 保存 server version。
	buf = pos + 1;                                              // 跳过 \0。

	connection_id_ = uint4korr(buf);                             // 读取 connection id。
	buf += 4;                                                   // 后移 4 字节。
	memcpy(auth_plugin_data_, buf, 8);                           // 读取 seed 前 8 字节。
	buf += 9;                                                   // 跳过 seed 前 8 字节和 filler。
	cap_flags_lower = uint2korr(buf);                            // 读取能力低位。
	buf += 2;                                                   // 后移。
	character_set_ = *buf++;                                    // 读取字符集。
	status_flags_ = uint2korr(buf);                              // 读取状态位。
	buf += 2;                                                   // 后移。
	cap_flags_upper = uint2korr(buf);                            // 读取能力高位。
	buf += 2;                                                   // 后移。
	capability_flags_ = (cap_flags_upper << 16U) + cap_flags_lower; // 合并完整能力位。
	auth_plugin_data_len_ = *buf++;                              // 读取 auth plugin data 长度。
	buf += 10;                                                  // 跳过 10 字节保留字段。
	if (auth_plugin_data_len_ > 21)                              // 源码假设 auth_plugin_data 固定 20 字节。
		return -2;                                             // 过长视为不完整/异常。

	memcpy(auth_plugin_data_ + 8, buf, 12);                      // 读取 seed 剩余 12 字节。
	buf += 13;                                                  // 跳过 seed 和终止 0。
	if (capability_flags_ & MYSQL_CAPFLAG_CLIENT_PLUGIN_AUTH)    // 如果有插件认证名。
	{
		if (buf == end || *(end - 1) != '\0')                   // 插件名必须以 \0 结尾。
			return -2;                                         // 不完整。

		auth_plugin_name_.assign((const char *)buf, end - 1 - buf); // 保存认证插件名。
	}

	return 1;                                                   // 握手包解析完成。
}

static std::string __native_password_encrypt(const std::string& password, // password：用户密码。
											 unsigned char seed[20])       // seed：服务端随机种子。
{
	unsigned char buf1[20];                                     // buf1：中间 SHA1 结果。
	unsigned char buf2[40];                                     // buf2：seed + hash 的临时缓冲。
	int i;                                                      // i：循环变量。

	// SHA1( password ) ^ SHA1( seed + SHA1( SHA1( password ) ) )
	SHA1((unsigned char *)password.c_str(), password.size(), buf1); // buf1 = SHA1(password)。
	SHA1(buf1, 20, buf2 + 20);                                  // buf2+20 = SHA1(SHA1(password))。
	memcpy(buf2, seed, 20);                                     // buf2 前 20 字节写 seed。
	SHA1(buf2, 40, buf2);                                       // buf2 = SHA1(seed + SHA1(SHA1(password)))。
	for (i = 0; i < 20; i++)                                    // 遍历 20 字节。
		buf1[i] ^= buf2[i];                                     // 异或得到认证响应。

	return std::string((const char *)buf1, 20);                 // 返回二进制认证串。
}

static std::string __caching_sha2_password_encrypt(const std::string& password, // MySQL 8 caching_sha2_password 加密。
												   unsigned char seed[20])       // seed：服务端随机种子。
{
	unsigned char buf1[32];                                     // buf1：SHA256(password)。
	unsigned char buf2[52];                                     // buf2：中间缓冲。
	int i;                                                      // i：循环变量。

	// SHA256( password ) ^ SHA256( SHA256( SHA256( password ) ) + seed)
	SHA256((unsigned char *)password.c_str(), password.size(), buf1); // buf1 = SHA256(password)。
	SHA256(buf1, 32, buf2);                                     // buf2 = SHA256(SHA256(password))。
	memcpy(buf2 + 32, seed, 20);                                // 拼接 seed。
	SHA256(buf2, 52, buf2);                                     // buf2 = SHA256(hash2 + seed)。
	for (i = 0; i < 32; i++)                                    // 遍历 32 字节。
		buf1[i] ^= buf2[i];                                     // 异或得到认证响应。

	return std::string((const char *)buf1, 32);                 // 返回认证串。
}

int MySQLAuthRequest::encode(struct iovec vectors[], int max)   // MySQLAuthRequest::encode：编码客户端认证请求。
{
	unsigned char header[32] = {0};                             // header：MySQL 认证包固定头部区域。
	unsigned char *pos = header;                                // pos：写入游标。
	std::string str;                                            // str：加密后的密码字段。

	int4store(pos, MYSQL_CAPFLAG_CLIENT_PROTOCOL_41 |            // 写客户端能力位。
				   MYSQL_CAPFLAG_CLIENT_SECURE_CONNECTION |
				   MYSQL_CAPFLAG_CLIENT_CONNECT_WITH_DB |
				   MYSQL_CAPFLAG_CLIENT_MULTI_RESULTS|
				   MYSQL_CAPFLAG_CLIENT_LOCAL_FILES |
				   MYSQL_CAPFLAG_CLIENT_MULTI_STATEMENTS |
				   MYSQL_CAPFLAG_CLIENT_PS_MULTI_RESULTS |
				   MYSQL_CAPFLAG_CLIENT_PLUGIN_AUTH);
	pos += 4;                                                   // 游标后移 4 字节。
	int4store(pos, 0);                                          // max packet size，这里写 0。
	pos += 4;                                                   // 游标后移。
	*pos = (uint8_t)character_set_;                             // 写字符集。

	if (password_.empty())                                      // 空密码。
		str.push_back(0);                                       // 密码字段长度为 0。
	else if (auth_plugin_name_ == CACHING_SHA2_PASSWORD)         // MySQL 8 认证插件。
	{
		str.push_back(32);                                      // caching_sha2 加密结果 32 字节。
		str += __caching_sha2_password_encrypt(password_, seed_); // 写加密密码。
	}
	else                                                        // 默认 native password。
	{
		str.push_back(20);                                      // native 加密结果 20 字节。
		str += __native_password_encrypt(password_, seed_);      // 写加密密码。
	}

	buf_.clear();                                               // 清空 payload。
	buf_.append((char *)header, 32);                             // 写固定 32 字节 header。
	buf_.append(username_.c_str(), username_.size() + 1);        // 写用户名，以 \0 结尾。
	buf_.append(str);                                           // 写密码认证字段。
	buf_.append(db_.c_str(), db_.size() + 1);                    // 写数据库名，以 \0 结尾。
	if (auth_plugin_name_.size() != 0)                           // 如果认证插件名非空。
		buf_.append(auth_plugin_name_.c_str(), auth_plugin_name_.size() + 1); // 写插件名。

	return MySQLMessage::encode(vectors, max);                  // 交给通用 MySQL packet 编码。
}

int MySQLAuthResponse::decode_packet(const unsigned char *buf, size_t buflen) // 解析认证响应。
{
	const unsigned char *end = buf + buflen;                    // end：输入末尾。
	const unsigned char *pos;                                   // pos：游标。
	const unsigned char *str;                                   // str：长度编码字符串起点。
	unsigned long long len;                                     // len：长度编码字符串长度。

	if (end == buf)                                             // 空包不完整。
		return -2;                                             // 返回 -2。

	switch (*buf)                                               // 根据第一个字节判断认证响应类型。
	{
	case 0x00:                                                  // OK packet。
	case 0xff:                                                  // ERROR packet。
		return MySQLResponse::decode_packet(buf, buflen);        // 交给普通响应解析。

	case 0xfe:                                                  // auth switch request。
		pos = ++buf;                                           // 跳过 0xfe。
		while (pos < end && *pos)                               // 查找插件名结束。
			pos++;

		if (pos >= end)                                        // 插件名不完整。
			return -2;                                         // 返回不完整。

		auth_plugin_name_.assign((const char *)buf, pos - buf);  // 保存服务端要求切换的认证插件。
		buf = pos + 1;                                          // 跳过 \0。
		if (buf == end || *(end - 1) != '\0')                   // seed 必须以 \0 结尾。
			return -2;

		if (end - 1 - buf != 20)                                // Workflow 这里要求 seed 为 20 字节。
			return -2;

		memcpy(seed_, buf, 20);                                 // 保存新 seed。
		return 1;                                              // 认证切换包解析完成。

	default:                                                    // 其他认证插件响应。
		pos = buf;                                             // 从头开始解析长度编码字符串。
		if (decode_string(&str, &len, &pos, end) > 0 && len == 1) // 解析一段长度为 1 的控制字段。
		{
			if (*str == 0x03)                                   // fast auth success。
			{
				if (end > pos)                                  // 如果后面还跟着 OK 包。
					return MySQLResponse::decode_packet(pos, end - pos); // 解析 OK 包。
				else
					return 0;                                  // 还需要继续接收 OK 包。
			}
			else if (*str == 0x04)                              // full auth required。
			{
				continue_ = true;                               // 标记需要继续认证。
				return 1;                                      // 当前包解析完成。
			}
		}

		return -2;                                             // 未识别或不完整。
	}
}

int MySQLAuthSwitchRequest::encode(struct iovec vectors[], int max) // 编码认证切换响应。
{
	if (password_.empty())                                      // 空密码。
	{
		buf_ = "\0";                                            // 发送单个 \0。
	}
	else if (auth_plugin_name_ == MYSQL_NATIVE_PASSWORD)         // native password。
	{
		buf_ = __native_password_encrypt(password_, seed_);      // 生成 native 密码响应。
	}
	else if (auth_plugin_name_ == CACHING_SHA2_PASSWORD)         // caching_sha2。
	{
		buf_ = __caching_sha2_password_encrypt(password_, seed_); // 生成 sha2 密码响应。
	}
	else if (auth_plugin_name_ == MYSQL_CLEAR_PASSWORD)          // clear password。
	{
		buf_ = password_;                                       // 明文密码。
		buf_.push_back('\0');                                   // 以 \0 结尾。
	}
	else                                                        // 不支持的插件。
	{
		errno = EINVAL;                                         // 参数无效。
		return -1;                                             // 编码失败。
	}

	return MySQLMessage::encode(vectors, max);                  // 编成 MySQL packet。
}

void MySQLResponse::set_ok_packet()                             // set_ok_packet()：构造一个简单 OK 包。
{
	uint16_t zero16 = 0;                                        // zero16：两字节 0。
	buf_.clear();                                               // 清空 payload。
	buf_.push_back(0x00);                                       // 0x00 表示 OK packet。
	buf_.append((const char *)&zero16, 2);                       // affected rows = 0。
	buf_.append((const char *)&zero16, 2);                       // last insert id = 0。
	buf_.append((const char *)&zero16, 2);                       // status/warnings 等简化字段。
}

int MySQLResponse::decode_packet(const unsigned char *buf, size_t buflen) // decode_packet：解析普通 MySQL 响应包。
{
	return mysql_parser_parse(buf, buflen, parser_);             // 交给底层 MySQL parser。
}

unsigned long long MySQLResponse::get_affected_rows() const     // get_affected_rows()：获取所有结果集影响行数之和。
{
	unsigned long long affected_rows = 0;                       // affected_rows：累计值。
	MySQLResultCursor cursor(this);                             // cursor：结果集游标。

	do {                                                        // 遍历所有 result set。
		affected_rows += cursor.get_affected_rows();             // 累加当前结果集影响行数。
	} while (cursor.next_result_set());                         // 切到下一个结果集。

	return affected_rows;                                       // 返回总影响行数。
}

unsigned long long MySQLResponse::get_last_insert_id() const    // get_last_insert_id()：获取最后一次插入 id。
{
	unsigned long long insert_id = 0;                           // insert_id：保存非零 insert id。
	MySQLResultCursor cursor(this);                             // cursor：结果集游标。

	do {                                                        // 遍历所有结果集。
		if (cursor.get_insert_id())                             // 如果当前结果集有 insert id。
			insert_id = cursor.get_insert_id();                  // 更新为当前 insert id。
	} while (cursor.next_result_set());                         // 继续下一个结果集。

	return insert_id;                                           // 返回最后一个非零 insert id。
}

int MySQLResponse::get_warnings() const                         // get_warnings()：统计 warning 数量。
{
	int warning_count = 0;                                      // warning_count：累计 warning。
	MySQLResultCursor cursor(this);                             // cursor：结果集游标。

	do {                                                        // 遍历所有结果集。
		warning_count += cursor.get_warnings();                  // 累加 warning。
	} while (cursor.next_result_set());                         // 继续下一个结果集。

	return warning_count;                                       // 返回 warning 总数。
}

std::string MySQLResponse::get_info() const                     // get_info()：拼接所有结果集 info。
{
	std::string info;                                           // info：输出信息。
	MySQLResultCursor cursor(this);                             // cursor：结果集游标。

	do {                                                        // 遍历所有结果集。
		if (info.length() > 0)                                  // 如果之前已经有内容。
			info += " ";                                        // 用空格分隔。
		info += cursor.get_info();                              // 拼接当前结果集 info。
	} while (cursor.next_result_set());                         // 继续下一个结果集。

	return info;                                                // 返回拼接结果。
}

bool MySQLResponse::is_ok_packet() const                        // is_ok_packet()：判断是否 OK 包。
{
	return parser_->packet_type == MYSQL_PACKET_OK;              // 底层 parser packet_type 等于 OK。
}

bool MySQLResponse::is_error_packet() const                     // is_error_packet()：判断是否 ERROR 包。
{
	return parser_->packet_type == MYSQL_PACKET_ERROR;           // 底层 parser packet_type 等于 ERROR。
}

/*
  说明：
  原始 MySQLMessage.cc 里还有 SSLRequest、PublicKeyResponse、RSAAuthRequest 等认证细节。
  学习随机图库项目时，优先掌握 set_query、encode、append、MySQLResponse 查询结果读取即可。
  真正写业务时通常使用：

  auto task = WFTaskFactory::create_mysql_task(url, retry, callback);
  task->get_req()->set_query("SELECT ...");
*/

}                                                              // namespace protocol

