#include <mysql/mysql.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <list>
#include <pthread.h>
#include <iostream>
#include "sql_connection_pool.h"

using namespace std;

/*
作用：
    构造数据库连接池对象并初始化计数器。
输入：
    无。
输出：
    无。
*/
connection_pool::connection_pool()
{
	m_CurConn = 0;
	m_FreeConn = 0;
}

/*
作用：
    返回数据库连接池单例。
输入：
    无。
输出：
    connection_pool*：全局唯一连接池实例。
*/
connection_pool *connection_pool::GetInstance()
{
	static connection_pool connPool;
	// 整个进程只保留一个数据库连接池实例，避免多处重复维护连接资源。
	return &connPool;
}

//构造初始化
/*
作用：
    初始化数据库连接池，预先创建多条 MySQL 连接。
输入：
    url：数据库地址。
    User：用户名。
    PassWord：密码。
    DBName：数据库名。
    Port：端口。
    MaxConn：连接池最大连接数。
    close_log：日志开关。
输出：
    无。
*/
void connection_pool::init(string url, string User, string PassWord, string DBName, int Port, int MaxConn, int close_log)
{
	// 这里不是按需建连，而是启动时一次性预创建好 MaxConn 条连接。
	m_url = url;
	m_Port = Port;
	m_User = User;
	m_PassWord = PassWord;
	m_DatabaseName = DBName;
	m_close_log = close_log;

	for (int i = 0; i < MaxConn; i++)
	{
		MYSQL *con = NULL;
		// mysql_init 只负责初始化连接句柄，本身还没真正连上数据库。
		con = mysql_init(con);

		if (con == NULL)
		{
			LOG_ERROR("MySQL Error");
			exit(1);
		}
		// mysql_real_connect 才是真正建立数据库连接的地方。
		// 参数依次是：连接句柄、主机、用户名、密码、数据库名、端口。
		con = mysql_real_connect(con, url.c_str(), User.c_str(), PassWord.c_str(), DBName.c_str(), Port, NULL, 0);

		if (con == NULL)
		{
			LOG_ERROR("MySQL Error");
			exit(1);
		}
		// 新建好的连接先放进空闲链表，表示此时可被业务线程借用。
		connList.push_back(con);
		++m_FreeConn;
	}

	// reserve 是信号量，初值等于空闲连接数。
	reserve = sem(m_FreeConn);

	m_MaxConn = m_FreeConn;
}


//当有请求时，从数据库连接池中返回一个可用连接，更新使用和空闲连接数
/*
作用：
    从连接池中借出一个空闲连接。
输入：
    无。
输出：
    MYSQL*：可用连接；如果连接池为空则返回 NULL。
*/
MYSQL *connection_pool::GetConnection()
{
	MYSQL *con = NULL;

	if (0 == connList.size())
		return NULL;

	// 信号量代表“当前还有多少空闲连接可借”，没有资源时线程会阻塞等待。
	// P 操作：没有空闲连接时，这里会阻塞等待。
	reserve.wait();
	
	lock.lock();

	// 从空闲连接链表头部取一个连接出来。
	con = connList.front();
	connList.pop_front();

	--m_FreeConn;
	++m_CurConn;

	lock.unlock();
	return con;
}

//释放当前使用的连接
/*
作用：
    把一个使用完毕的连接归还到连接池。
输入：
    con：待归还的连接。
输出：
    true：归还成功。
    false：输入为空。
*/
bool connection_pool::ReleaseConnection(MYSQL *con)
{
	if (NULL == con)
		return false;

	lock.lock();

	// 把用完的连接重新挂回空闲链表。
	// 归还时不关闭连接，而是重新放回空闲链表，等待下次复用。
	connList.push_back(con);
	++m_FreeConn;
	--m_CurConn;

	lock.unlock();

	// 归还连接后递增信号量，唤醒其他可能正在等待连接的线程。
	// V 操作：告诉其他线程“现在多了一个可用连接”。
	reserve.post();
	return true;
}

//销毁数据库连接池
/*
作用：
    销毁连接池中的所有连接。
输入：
    无。
输出：
    无。
*/
void connection_pool::DestroyPool()
{

	lock.lock();
	if (connList.size() > 0)
	{
		list<MYSQL *>::iterator it;
		for (it = connList.begin(); it != connList.end(); ++it)
		{
			MYSQL *con = *it;
			// 服务整体退出时，才真正关闭底层 MySQL 连接。
			mysql_close(con);
		}
		m_CurConn = 0;
		m_FreeConn = 0;
		connList.clear();
	}

	lock.unlock();
}

//当前空闲的连接数
/*
作用：
    获取当前空闲连接数量。
输入：
    无。
输出：
    int：空闲连接数。
*/
int connection_pool::GetFreeConn()
{
	return this->m_FreeConn;
}

/*
作用：
    析构连接池对象。
输入：
    无。
输出：
    无。
*/
connection_pool::~connection_pool()
{
	DestroyPool();
}

/*
作用：
    RAII 封装：构造时自动获取一个连接。
输入：
    SQL：输出参数，用于接收借到的 MYSQL*。
    connPool：连接池对象。
输出：
    无。
*/
connectionRAII::connectionRAII(MYSQL **SQL, connection_pool *connPool){
	// RAII 的关键点：构造时拿连接，析构时自动归还，避免忘记释放。
	*SQL = connPool->GetConnection();
	
	conRAII = *SQL;
	poolRAII = connPool;
}

/*
作用：
    RAII 封装：析构时自动把连接归还给连接池。
输入：
    无。
输出：
    无。
*/
connectionRAII::~connectionRAII(){
	poolRAII->ReleaseConnection(conRAII);
}
