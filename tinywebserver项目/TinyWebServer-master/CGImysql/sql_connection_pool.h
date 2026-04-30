#ifndef _CONNECTION_POOL_
#define _CONNECTION_POOL_

#include <stdio.h>
#include <list>
#include <mysql/mysql.h>
#include <error.h>
#include <string.h>
#include <iostream>
#include <string>
#include "../lock/locker.h"
#include "../log/log.h"

using namespace std;

class connection_pool
{
public:
    /*
    作用：
        从连接池中取出一个可用的 MySQL 连接。
    输入：
        无。
    输出：
        成功返回 MYSQL*，如果连接池为空则返回 NULL。
    */
    MYSQL *GetConnection();

    /*
    作用：
        把一个已经使用完成的连接归还到连接池。
    输入：
        conn：待归还的 MySQL 连接。
    输出：
        true 表示归还成功，false 表示输入连接为空。
    */
    bool ReleaseConnection(MYSQL *conn);

    /*
    作用：
        获取当前空闲连接数量。
    输入：
        无。
    输出：
        返回当前空闲连接数。
    */
    int GetFreeConn();

    /*
    作用：
        销毁连接池中的全部连接。
    输入：
        无。
    输出：
        无。
    */
    void DestroyPool();

    // 单例入口：整个进程只维护一个数据库连接池。
    static connection_pool *GetInstance();

    /*
    作用：
        初始化数据库连接池，预先创建多条 MySQL 连接。
    输入：
        url：数据库主机地址。
        User：数据库用户名。
        PassWord：数据库密码。
        DataBaseName：数据库名。
        Port：数据库端口。
        MaxConn：连接池最大连接数。
        close_log：日志开关。
    输出：
        无。
    */
    void init(string url, string User, string PassWord, string DataBaseName, int Port, int MaxConn, int close_log);

private:
    connection_pool();
    ~connection_pool();

    // 连接池允许维护的最大连接数。
    int m_MaxConn;
    // 当前正在被业务线程使用的连接数。
    int m_CurConn;
    // 当前处于空闲状态、可被借出的连接数。
    int m_FreeConn;
    // 保护连接链表和计数器的互斥锁。
    locker lock;
    // 空闲连接链表，链表中每个 MYSQL* 都表示一个可复用连接。
    list<MYSQL *> connList;
    // 信号量：表示当前还剩多少个可用连接。
    sem reserve;

public:
    // 数据库主机地址。
    string m_url;
    // 数据库端口，源码中虽然声明成 string，但实际主要在初始化时记录信息。
    string m_Port;
    // 数据库用户名。
    string m_User;
    // 数据库密码。
    string m_PassWord;
    // 数据库名称。
    string m_DatabaseName;
    // 是否关闭日志。
    int m_close_log;
};

class connectionRAII
{
public:
    /*
    作用：
        构造时自动从连接池中借一个连接。
    输入：
        con：输出参数，用于接收借到的 MYSQL*。
        connPool：连接池对象。
    输出：
        无。
    */
    connectionRAII(MYSQL **con, connection_pool *connPool);

    /*
    作用：
        析构时自动把连接归还到连接池。
    输入：
        无。
    输出：
        无。
    */
    ~connectionRAII();

private:
    // 当前这次 RAII 生命周期持有的 MySQL 连接。
    MYSQL *conRAII;
    // 当前连接所属的连接池对象。
    connection_pool *poolRAII;
};

#endif
