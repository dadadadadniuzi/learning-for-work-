/*
  注释版源码文件：MySQLResult.h

  原始文件位置：
  workflow-master/src/protocol/MySQLResult.h

  本文件用途：
  提供 MySQL 查询结果读取工具。

  对随机图库项目最重要：
  1. MySQLResponse 只代表一次 MySQL 响应消息。
  2. 如果 SQL 是 SELECT，需要用 MySQLResultCursor 读取结果集。
  3. 结果集里的每个字段用 MySQLCell 表示。
  4. 字段元数据用 MySQLField 表示。

  示例：
      MySQLResultCursor cursor(task->get_resp());
      std::vector<MySQLCell> row;
      while (cursor.fetch_row(row)) {
          std::string path = row[2].as_string();
      }
*/

#ifndef _MYSQLRESULT_H_                                       // 头文件保护宏：防止重复 include。
#define _MYSQLRESULT_H_                                       // 定义头文件保护宏。

#include <map>                                                // map：支持按字段名读取一行数据。
#include <vector>                                             // vector：支持按列下标读取一行数据。
#include <string>                                             // string：字符串值和字段名。
#include <unordered_map>                                      // unordered_map：支持哈希表形式按字段名读取。
#include "mysql_types.h"                                      // MySQL 字段类型定义。
#include "mysql_parser.h"                                     // MySQL parser 和结果集结构。
#include "MySQLMessage.h"                                     // MySQLResponse 定义。

namespace protocol                                            // protocol 命名空间。
{

class MySQLCell                                               // MySQLCell：结果集中一个单元格的值。
{
public:
	MySQLCell();                                              // 默认构造，通常表示空单元格。

	MySQLCell(MySQLCell&& move);                              // 移动构造，接管单元格数据。
	MySQLCell& operator=(MySQLCell&& move);                    // 移动赋值。

	MySQLCell(const void *data, size_t len, int data_type);    // 构造函数：用原始数据、长度和 MySQL 类型创建单元格。

	int get_data_type() const;                                // get_data_type()：获取 MySQL 字段类型。

	bool is_null() const;                                     // is_null()：是否为 NULL。
	bool is_int() const;                                      // is_int()：是否可按 int 读取。
	bool is_string() const;                                   // is_string()：是否可按字符串读取。
	bool is_float() const;                                    // is_float()：是否为 float。
	bool is_double() const;                                   // is_double()：是否为 double。
	bool is_ulonglong() const;                                // is_ulonglong()：是否为 unsigned long long。
	bool is_date() const;                                     // is_date()：是否为日期。
	bool is_time() const;                                     // is_time()：是否为时间。
	bool is_datetime() const;                                 // is_datetime()：是否为日期时间。

	int as_int() const;                                       // as_int()：复制并转换为 int。
	std::string as_string() const;                            // as_string()：复制并转换为字符串。
	std::string as_binary_string() const;                     // as_binary_string()：按二进制字符串复制。
	float as_float() const;                                   // as_float()：转换为 float。
	double as_double() const;                                 // as_double()：转换为 double。
	unsigned long long as_ulonglong() const;                  // as_ulonglong()：转换为 unsigned long long。
	std::string as_date() const;                              // as_date()：转换为日期字符串。
	std::string as_time() const;                              // as_time()：转换为时间字符串。
	std::string as_datetime() const;                          // as_datetime()：转换为日期时间字符串。

	void get_cell_nocopy(const void **data, size_t *len, int *data_type) const; // 不复制地获取原始数据指针、长度和类型。

private:
	int data_type;                                            // data_type：MySQL 字段类型。
	void *data;                                               // data：单元格数据地址。
	size_t len;                                               // len：数据长度。
};

class MySQLField                                              // MySQLField：一列字段的元数据。
{
public:
	MySQLField(const void *buf, mysql_field_t *field);         // 构造函数：从 parser 的字段结构初始化。

	std::string get_name() const;                             // get_name()：获取列名。
	std::string get_org_name() const;                         // get_org_name()：获取原始列名，处理别名时有用。
	std::string get_table() const;                            // get_table()：获取表名。
	std::string get_org_table() const;                        // get_org_table()：获取原始表名。
	std::string get_db() const;                               // get_db()：获取数据库名。
	std::string get_catalog() const;                          // get_catalog()：获取 catalog。
	std::string get_def() const;                              // get_def()：获取默认值。
	int get_charsetnr() const;                                // get_charsetnr()：获取字符集编号。
	int get_length() const;                                   // get_length()：获取列定义长度。
	int get_flags() const;                                    // get_flags()：获取字段 flags。
	int get_decimals() const;                                 // get_decimals()：获取小数位数。
	int get_data_type() const;                                // get_data_type()：获取字段类型。

private:
	const char *name;                                         // name：列名。
	const char *org_name;                                     // org_name：原始列名。
	const char *table;                                        // table：表名。
	const char *org_table;                                    // org_table：原始表名。
	const char *db;                                           // db：数据库名。
	const char *catalog;                                      // catalog：catalog 名。
	const char *def;                                          // def：默认值。
	int length;                                               // length：列定义长度。
	int name_length;                                          // name_length：列名长度。
	int org_name_length;                                      // org_name_length：原始列名长度。
	int table_length;                                         // table_length：表名长度。
	int org_table_length;                                     // org_table_length：原始表名长度。
	int db_length;                                            // db_length：数据库名长度。
	int catalog_length;                                       // catalog_length：catalog 长度。
	int def_length;                                           // def_length：默认值长度。
	int flags;                                                // flags：字段标志位。
	int decimals;                                             // decimals：小数位数。
	int charsetnr;                                            // charsetnr：字符集编号。
	int data_type;                                            // data_type：字段类型。
};

class MySQLResultCursor                                       // MySQLResultCursor：结果集游标，用来遍历 SELECT 返回结果。
{
public:
	MySQLResultCursor(const MySQLResponse *resp);              // 构造函数：从 MySQLResponse 创建游标。

	MySQLResultCursor(MySQLResultCursor&& move);               // 移动构造。
	MySQLResultCursor& operator=(MySQLResultCursor&& move);    // 移动赋值。

	virtual ~MySQLResultCursor();                              // 析构函数：释放字段数组等内部资源。

	bool next_result_set();                                   // next_result_set()：切换到下一个结果集，多语句查询时有用。
	void first_result_set();                                  // first_result_set()：回到第一个结果集。

	const MySQLField *fetch_field();                          // fetch_field()：逐个读取字段元数据。
	const MySQLField *const *fetch_fields() const;             // fetch_fields()：一次性获取字段数组。

	bool fetch_row(std::vector<MySQLCell>& row_arr);           // fetch_row(row_arr)：按列下标读取一行。
	bool fetch_row(std::map<std::string, MySQLCell>& row_map); // fetch_row(row_map)：按字段名读取一行，使用有序 map。
	bool fetch_row(std::unordered_map<std::string, MySQLCell>& row_map); // 按字段名读取一行，使用 unordered_map。

	bool fetch_row_nocopy(const void **data, size_t *len, int *data_type); // 不复制地读取当前行单元格数据。
	bool fetch_all(std::vector<std::vector<MySQLCell>>& rows); // fetch_all(rows)：读取全部行到二维数组。

	int get_cursor_status() const;                            // get_cursor_status()：获取游标状态。
	int get_server_status() const;                            // get_server_status()：获取 MySQL server status。

	int get_field_count() const;                              // get_field_count()：列数。
	int get_rows_count() const;                               // get_rows_count()：行数。
	unsigned long long get_affected_rows() const;              // get_affected_rows()：影响行数。
	unsigned long long get_insert_id() const;                  // get_insert_id()：插入 id。
	int get_warnings() const;                                 // get_warnings()：warning 数量。
	std::string get_info() const;                             // get_info()：额外信息。

	void rewind();                                            // rewind()：游标回到结果集开头。

public:
	MySQLResultCursor();                                      // 默认构造，之后可 reset。
	void reset(MySQLResponse *resp);                           // reset(resp)：重新绑定一个响应。

private:
	void init(const MySQLResponse *resp);                      // init(resp)：用响应初始化游标。
	void init();                                              // init()：初始化内部状态。
	void clear();                                             // clear()：清理内部状态。

	void fetch_result_set(const struct __mysql_result_set *result_set); // 从底层结果集结构提取字段/行信息。

	template<class T>
	bool fetch_row(T& row_map);                               // 模板版本：填充 map/unordered_map。

	int status;                                               // status：游标状态。
	int server_status;                                        // server_status：MySQL 服务端状态。
	const void *start;                                        // start：结果数据起始地址。
	const void *end;                                          // end：结果数据结束地址。
	const void *pos;                                          // pos：当前读取位置。

	const void **row_data;                                    // row_data：当前行每列数据指针。
	MySQLField **fields;                                      // fields：字段元数据数组。
	int row_count;                                            // row_count：行数。
	int field_count;                                          // field_count：列数。
	int current_row;                                          // current_row：当前行下标。
	int current_field;                                        // current_field：当前字段下标。

	unsigned long long affected_rows;                         // affected_rows：影响行数。
	unsigned long long insert_id;                             // insert_id：自增 id。
	int warning_count;                                        // warning_count：warning 数量。
	int info_len;                                             // info_len：info 文本长度。

	mysql_result_set_cursor_t cursor;                         // cursor：底层 C 风格结果集游标。
	mysql_parser_t *parser;                                   // parser：来自 MySQLResponse 的 parser。
};

}                                                            // namespace protocol 结束。

#include "MySQLResult.inl"                                   // 引入内联实现。

#endif                                                       // 结束头文件保护宏。
