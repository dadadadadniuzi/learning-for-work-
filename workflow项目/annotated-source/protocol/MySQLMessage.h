/*
  注释版源码文件：MySQLMessage.h

  原始文件位置：
  workflow-master/src/protocol/MySQLMessage.h

  本文件用途：
  定义 Workflow 的 MySQL 协议对象。

  对随机图库项目最重要：
  1. MySQLRequest：设置 SQL，比如 INSERT 图片元数据、SELECT 图片路径。
  2. MySQLResponse：判断 SQL 执行是否成功，读取 OK 包、错误包信息。
  3. 真正读取 SELECT 结果集通常要配合 MySQLResult.h。

  示例：
      WFMySQLTask *task = WFTaskFactory::create_mysql_task(mysql_url, 0, callback);
      task->get_req()->set_query("SELECT id, filename, storage_path FROM images");
*/

#ifndef _MYSQLMESSAGE_H_                                      // 头文件保护宏：防止重复 include。
#define _MYSQLMESSAGE_H_                                      // 定义头文件保护宏。

#include <stdint.h>                                           // 引入 uint8_t 等固定宽度整数类型。
#include <string>                                             // 引入 std::string，用于 SQL 和错误信息。
#include "ProtocolMessage.h"                                  // 引入 ProtocolMessage，MySQLMessage 要实现 encode/append。
#include "mysql_stream.h"                                     // 引入 mysql_stream_t，用于接收 MySQL 数据流。
#include "mysql_parser.h"                                     // 引入 mysql_parser_t，用于解析 MySQL 协议包。

namespace protocol                                            // protocol 命名空间。
{

class MySQLMessage : public ProtocolMessage                    // MySQLMessage：MySQL 请求和响应的共同基类。
{
public:
	mysql_parser_t *get_parser() const;                        // get_parser()：获取底层 MySQL parser，结果集解析会用到。
	int get_seqid() const;                                     // get_seqid()：获取 MySQL 包序号。
	void set_seqid(int seqid);                                 // set_seqid(seqid)：设置 MySQL 包序号。
	int get_command() const;                                   // get_command()：获取 MySQL 命令类型，比如 COM_QUERY。

protected:
	virtual int append(const void *buf, size_t *size);          // append(buf, size)：接收网络字节并追加解析。
	virtual int encode(struct iovec vectors[], int max);        // encode(vectors, max)：把 MySQL 消息编码成 iovec。
	virtual int decode_packet(const unsigned char *buf, size_t buflen) { return 1; } // decode_packet：解析单个包，默认直接成功。

	void set_command(int cmd) const;                           // set_command(cmd)：设置 MySQL 命令类型。

	mysql_stream_t *stream_;                                   // stream_：MySQL 输入流，处理粘包/半包。
	mysql_parser_t *parser_;                                   // parser_：MySQL 协议解析器。

	unsigned char heads_[256][4];                              // heads_：编码时保存多个 MySQL 包的 4 字节包头。
	uint8_t seqid_;                                            // seqid_：当前包序号。
	std::string buf_;                                          // buf_：编码时保存要发送的数据体。
	size_t cur_size_;                                          // cur_size_：当前编码发送进度。

public:
	MySQLMessage();                                            // 构造函数：初始化 stream/parser/buffer。
	virtual ~MySQLMessage();                                   // 析构函数：释放 stream/parser。
	MySQLMessage(MySQLMessage&& move);                         // 移动构造：接管内部资源。
	MySQLMessage& operator= (MySQLMessage&& move);              // 移动赋值。
};

class MySQLRequest : public MySQLMessage                       // MySQLRequest：MySQL 请求对象。
{
public:
	void set_query(const char *query);                         // set_query(query)：设置 SQL，C 字符串版本。
	void set_query(const std::string& query);                  // set_query(query)：设置 SQL，std::string 版本。
	void set_query(const char *query, size_t length);           // set_query(query, length)：设置指定长度 SQL。

	std::string get_query() const;                             // get_query()：获取当前 SQL 字符串。
	bool query_is_unset() const;                               // query_is_unset()：判断 SQL 是否还没有设置。

public:
	MySQLRequest() = default;                                  // 默认构造。
	MySQLRequest(MySQLRequest&& move) = default;               // 默认移动构造。
	MySQLRequest& operator= (MySQLRequest&& move) = default;   // 默认移动赋值。
};

class MySQLResponse : public MySQLMessage                      // MySQLResponse：MySQL 响应对象。
{
public:
	bool is_ok_packet() const;                                 // is_ok_packet()：判断响应是否是 OK 包。
	bool is_error_packet() const;                              // is_error_packet()：判断响应是否是错误包。
	int get_packet_type() const;                               // get_packet_type()：获取响应包类型。

	unsigned long long get_affected_rows() const;              // get_affected_rows()：INSERT/UPDATE/DELETE 影响行数。
	unsigned long long get_last_insert_id() const;             // get_last_insert_id()：INSERT 后的自增 ID。
	int get_warnings() const;                                  // get_warnings()：获取 warning 数量。
	int get_error_code() const;                                // get_error_code()：获取 MySQL 错误码。
	std::string get_error_msg() const;                         // get_error_msg()：获取错误文本。
	std::string get_sql_state() const;                         // get_sql_state()：获取 SQLSTATE。
	std::string get_info() const;                              // get_info()：获取 OK 包中的 info 文本。

	void set_ok_packet();                                      // set_ok_packet()：服务端场景设置 OK 响应包。

public:
	MySQLResponse() = default;                                 // 默认构造。
	MySQLResponse(MySQLResponse&& move) = default;             // 默认移动构造。
	MySQLResponse& operator= (MySQLResponse&& move) = default; // 默认移动赋值。

protected:
	virtual int decode_packet(const unsigned char *buf, size_t buflen); // decode_packet：解析 MySQL 响应包。
};

}                                                             // namespace protocol 结束。

#include "MySQLMessage.inl"                                   // 引入内联实现；包含 getter/setter 等实现细节。

#endif                                                        // 结束头文件保护宏。
