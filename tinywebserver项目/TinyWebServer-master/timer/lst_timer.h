#ifndef LST_TIMER
#define LST_TIMER

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>

#include <time.h>
#include "../log/log.h"

class util_timer;

struct client_data
{
    // 客户端地址信息。
    sockaddr_in address;
    // 对应连接的 socket fd。
    int sockfd;
    // 指向该连接绑定的定时器节点。
    util_timer *timer;
};

class util_timer
{
public:
    util_timer() : prev(NULL), next(NULL) {}

public:
    // 这个定时器的过期时间点，单位是时间戳秒值。
    time_t expire;

    // 超时后要执行的回调函数。
    void (*cb_func)(client_data *);
    // 回调函数需要用到的连接数据。
    client_data *user_data;
    // 双向链表前驱指针。
    util_timer *prev;
    // 双向链表后继指针。
    util_timer *next;
};

class sort_timer_lst
{
public:
    // 作用：构造升序定时器链表。
    sort_timer_lst();
    // 作用：析构链表并释放所有定时器节点。
    ~sort_timer_lst();

    // 作用：把新定时器按过期时间插入到升序链表中。
    void add_timer(util_timer *timer);
    // 作用：连接活跃时调整定时器位置，延后过期时间。
    void adjust_timer(util_timer *timer);
    // 作用：删除指定定时器节点。
    void del_timer(util_timer *timer);
    // 作用：处理所有已经超时的定时器。
    void tick();

private:
    // 从某个位置开始查找，把 timer 插入到正确的升序位置。
    void add_timer(util_timer *timer, util_timer *lst_head);

    // 升序链表头指针，最早过期的节点总在前面。
    util_timer *head;
    // 升序链表尾指针，最晚过期的节点总在后面。
    util_timer *tail;
};

class Utils
{
public:
    Utils() {}
    ~Utils() {}

    // 作用：初始化时间槽大小，决定 alarm 的触发间隔。
    void init(int timeslot);

    // 作用：把 fd 设为非阻塞模式。
    int setnonblocking(int fd);

    // 作用：把 fd 注册到 epoll 中，并按需设置 ET / ONESHOT。
    void addfd(int epollfd, int fd, bool one_shot, int TRIGMode);

    // 作用：信号处理函数，把信号编号写进管道。
    static void sig_handler(int sig);

    // 作用：安装信号处理函数。
    void addsig(int sig, void(handler)(int), bool restart = true);

    // 作用：处理一次定时任务，并重新设置 alarm。
    void timer_handler();

    // 作用：向客户端写入简单错误提示并关闭连接。
    void show_error(int connfd, const char *info);

public:
    // 全局共享的信号管道数组地址。
    static int *u_pipefd;
    // 升序定时器链表对象。
    sort_timer_lst m_timer_lst;
    // 全局共享的 epoll fd。
    static int u_epollfd;
    // 时间槽大小，单位秒。
    int m_TIMESLOT;
};

// 作用：定时器超时回调，关闭连接并减少在线连接计数。
void cb_func(client_data *user_data);

#endif
