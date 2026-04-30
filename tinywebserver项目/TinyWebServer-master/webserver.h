#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <cassert>
#include <sys/epoll.h>

#include "./threadpool/threadpool.h"
#include "./http/http_conn.h"

// 进程最多维护多少个文件描述符/连接对象。
const int MAX_FD = 65536;
// epoll_wait 单轮最多取出的就绪事件数。
const int MAX_EVENT_NUMBER = 10000;
// 定时器时间槽，单位秒。
const int TIMESLOT = 5;

class WebServer
{
public:
    /*
    作用：
        构造服务器总控对象，分配连接数组、定时器数组等核心资源。
    输入：
        无。
    输出：
        无。
    */
    WebServer();

    /*
    作用：
        释放服务器生命周期内申请的资源，例如 epoll、监听 fd、线程池和连接数组。
    输入：
        无。
    输出：
        无。
    */
    ~WebServer();

    /*
    作用：
        保存服务器运行所需的基础配置，但此时还不真正启动各个子系统。
    输入：
        port：监听端口。
        user/passWord/databaseName：数据库连接信息。
        log_write：日志写入模式，同步或异步。
        opt_linger：是否启用优雅关闭。
        trigmode：LT/ET 触发模式组合。
        sql_num：数据库连接池大小。
        thread_num：线程池线程数。
        close_log：是否关闭日志。
        actor_model：并发模型，Reactor 或更接近 Proactor 的风格。
    输出：
        无。
    */
    void init(int port, string user, string passWord, string databaseName,
              int log_write, int opt_linger, int trigmode, int sql_num,
              int thread_num, int close_log, int actor_model);

    // 作用：创建线程池对象，为后续请求分发准备工作线程。
    void thread_pool();
    // 作用：初始化数据库连接池，并把已有用户信息预读到内存。
    void sql_pool();
    // 作用：初始化日志系统，决定是否启用异步日志。
    void log_write();
    // 作用：根据配置决定 listenfd 和 connfd 分别采用 LT 还是 ET。
    void trig_mode();
    // 作用：完成 socket、bind、listen、epoll、信号管道等底层初始化。
    void eventListen();
    // 作用：服务器主循环，统一处理连接、读写、信号和超时事件。
    void eventLoop();
    // 作用：为新连接创建 http_conn 对象并绑定定时器。
    void timer(int connfd, struct sockaddr_in client_address);
    // 作用：连接有活动时延长超时时间，防止活跃连接被误回收。
    void adjust_timer(util_timer *timer);
    // 作用：关闭超时或异常连接，并删除对应定时器。
    void deal_timer(util_timer *timer, int sockfd);
    // 作用：处理新客户端连接到来的情况。
    // 输出：true 表示本轮成功接收连接，false 表示接收失败或 ET 模式下队列已取空。
    bool dealclientdata();
    // 作用：处理信号管道中的数据，把信号转成 timeout / stop_server 标志位。
    // 输出：true 表示读取成功，false 表示读取失败。
    bool dealwithsignal(bool &timeout, bool &stop_server);
    // 作用：处理连接上的可读事件。
    void dealwithread(int sockfd);
    // 作用：处理连接上的可写事件。
    void dealwithwrite(int sockfd);

public:
    // 服务器监听端口。
    int m_port;
    // 网站根目录绝对路径，例如 .../TinyWebServer-master/root
    char *m_root;
    // 日志写入模式：
    // 0 表示同步日志
    // 1 表示异步日志
    int m_log_write;
    // 是否关闭日志功能。
    int m_close_log;
    // 并发模型选择：
    // 0 更接近 Proactor 风格
    // 1 Reactor 风格
    int m_actormodel;

    // 信号管道 fd 对：
    // m_pipefd[0] 是读端
    // m_pipefd[1] 是写端
    int m_pipefd[2];
    // 当前进程统一使用的 epoll 实例 fd。
    int m_epollfd;
    // 连接对象数组，下标通常直接使用 sockfd。
    http_conn *users;

    // 数据库连接池对象。
    connection_pool *m_connPool;
    // 数据库用户名。
    string m_user;
    // 数据库密码。
    string m_passWord;
    // 数据库名称。
    string m_databaseName;
    // 数据库连接池大小。
    int m_sql_num;

    // 线程池对象。
    threadpool<http_conn> *m_pool;
    // 线程池中的工作线程数。
    int m_thread_num;

    // epoll_wait 每轮返回的就绪事件数组。
    epoll_event events[MAX_EVENT_NUMBER];

    // 监听 socket fd。
    int m_listenfd;
    // linger 配置：是否启用优雅关闭。
    int m_OPT_LINGER;
    // 总的触发模式组合配置值。
    int m_TRIGMode;
    // 监听 fd 的触发模式：LT 或 ET。
    int m_LISTENTrigmode;
    // 已连接 fd 的触发模式：LT 或 ET。
    int m_CONNTrigmode;

    // 每个连接对应的一份定时器辅助数据数组。
    client_data *users_timer;
    // 工具类对象，封装定时器链表、信号处理、epoll 注册等公共逻辑。
    Utils utils;
};

#endif
