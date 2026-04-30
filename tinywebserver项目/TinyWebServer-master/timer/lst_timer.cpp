#include "lst_timer.h"
#include "../http/http_conn.h"

/*
作用：
    构造一个按过期时间升序排列的双向链表。
输入：
    无。
输出：
    无。
*/
sort_timer_lst::sort_timer_lst()
{
    head = NULL;
    tail = NULL;
}
/*
作用：
    析构定时器链表并释放所有节点。
输入：
    无。
输出：
    无。
*/
sort_timer_lst::~sort_timer_lst()
{
    util_timer *tmp = head;
    while (tmp)
    {
        head = tmp->next;
        delete tmp;
        tmp = head;
    }
}

/*
作用：
    将一个新的定时器插入到有序链表中。
输入：
    timer：待插入的定时器。
输出：
    无。
*/
void sort_timer_lst::add_timer(util_timer *timer)
{
    if (!timer)
    {
        return;
    }
    if (!head)
    {
        head = tail = timer;
        return;
    }
    // 如果比当前头节点更早过期，就直接插到链表头。
    if (timer->expire < head->expire)
    {
        timer->next = head;
        head->prev = timer;
        head = timer;
        return;
    }
    add_timer(timer, head);
}
/*
作用：
    当连接重新活跃时，调整它的定时器在链表中的位置。
输入：
    timer：需要调整的定时器。
输出：
    无。
*/
void sort_timer_lst::adjust_timer(util_timer *timer)
{
    if (!timer)
    {
        return;
    }
    // 这里只处理“超时时间往后延”的情况，所以只需要和后继节点比较。
    util_timer *tmp = timer->next;
    if (!tmp || (timer->expire < tmp->expire))
    {
        return;
    }
    if (timer == head)
    {
        head = head->next;
        head->prev = NULL;
        timer->next = NULL;
        add_timer(timer, head);
    }
    else
    {
        timer->prev->next = timer->next;
        timer->next->prev = timer->prev;
        add_timer(timer, timer->next);
    }
}
/*
作用：
    从链表中删除一个定时器节点。
输入：
    timer：待删除的定时器。
输出：
    无。
*/
void sort_timer_lst::del_timer(util_timer *timer)
{
    if (!timer)
    {
        return;
    }
    if ((timer == head) && (timer == tail))
    {
        delete timer;
        head = NULL;
        tail = NULL;
        return;
    }
    if (timer == head)
    {
        head = head->next;
        head->prev = NULL;
        delete timer;
        return;
    }
    if (timer == tail)
    {
        tail = tail->prev;
        tail->next = NULL;
        delete timer;
        return;
    }
    timer->prev->next = timer->next;
    timer->next->prev = timer->prev;
    delete timer;
}
/*
作用：
    扫描并处理所有已经超时的连接。
输入：
    无。
输出：
    无。
说明：
    因为链表按过期时间升序排列，所以一旦遇到一个“还没过期”的节点，就可以立刻停止扫描。
*/
void sort_timer_lst::tick()
{
    if (!head)
    {
        return;
    }
    
    // time(NULL) 返回当前 Unix 时间戳（秒）。
    time_t cur = time(NULL);
    util_timer *tmp = head;
    while (tmp)
    {
        if (cur < tmp->expire)
        {
            break;
        }
        // 调用超时回调，真正执行关闭连接等清理逻辑。
        tmp->cb_func(tmp->user_data);
        head = tmp->next;
        if (head)
        {
            head->prev = NULL;
        }
        delete tmp;
        tmp = head;
    }
}

/*
作用：
    从某个链表位置开始，把定时器插入到正确的升序位置。
输入：
    timer：待插入节点。
    lst_head：查找插入位置的起点。
输出：
    无。
*/
void sort_timer_lst::add_timer(util_timer *timer, util_timer *lst_head)
{
    util_timer *prev = lst_head;
    util_timer *tmp = prev->next;
    while (tmp)
    {
        if (timer->expire < tmp->expire)
        {
            // 找到第一个过期时间比自己晚的节点，就插到它前面。
            prev->next = timer;
            timer->next = tmp;
            tmp->prev = timer;
            timer->prev = prev;
            break;
        }
        // 当前位置还不合适，继续向后找。
        prev = tmp;
        tmp = tmp->next;
    }
    if (!tmp)
    {
        // 一直走到链表尾部还没插进去，说明它是当前最晚过期的节点。
        prev->next = timer;
        timer->prev = prev;
        timer->next = NULL;
        tail = timer;
    }
}

/*
作用：
    记录时间槽大小。
输入：
    timeslot：每次 alarm 的触发间隔，单位秒。
输出：
    无。
*/
void Utils::init(int timeslot)
{
    m_TIMESLOT = timeslot;
}

//对文件描述符设置非阻塞
int Utils::setnonblocking(int fd)
{
    // fcntl 先取旧标志，再补上 O_NONBLOCK，再写回。
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

//将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
void Utils::addfd(int epollfd, int fd, bool one_shot, int TRIGMode)
{
    epoll_event event;
    event.data.fd = fd;

    if (1 == TRIGMode)
        event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    else
        event.events = EPOLLIN | EPOLLRDHUP;

    if (one_shot)
        event.events |= EPOLLONESHOT;
    // EPOLL_CTL_ADD 表示把 fd 注册进 epoll 关注列表。
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

//信号处理函数
/*
作用：
    信号处理函数，把信号值写入管道，让主线程在 epoll 中统一处理。
输入：
    sig：收到的信号编号。
输出：
    无。
*/
void Utils::sig_handler(int sig)
{
    //为保证函数的可重入性，保留原来的errno
    int save_errno = errno;
    int msg = sig;
    // send 把信号编号写进管道，让主线程在 epoll 中统一处理。
    send(u_pipefd[1], (char *)&msg, 1, 0);
    errno = save_errno;
}

//设置信号函数
void Utils::addsig(int sig, void(handler)(int), bool restart)
{
    struct sigaction sa;
    // sigaction 是更标准、更可靠的信号安装方式。
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;
    if (restart)
        sa.sa_flags |= SA_RESTART;
    // 处理该信号期间，先临时屏蔽其他信号，降低重入干扰。
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}

//定时处理任务，重新定时以不断触发SIGALRM信号
/*
作用：
    定时任务入口，处理一次超时扫描，并重新设置下一次 alarm。
输入：
    无。
输出：
    无。
*/
void Utils::timer_handler()
{
    // 统一处理所有已超时连接。
    m_timer_lst.tick();
    // 重新预约下一次 SIGALRM。
    alarm(m_TIMESLOT);
}

void Utils::show_error(int connfd, const char *info)
{
    // 先把错误信息发给客户端，再关闭连接。
    send(connfd, info, strlen(info), 0);
    close(connfd);
}

int *Utils::u_pipefd = 0;
int Utils::u_epollfd = 0;

class Utils;
/*
作用：
    定时器超时回调，关闭一个失活连接。
输入：
    user_data：连接对应的客户端数据。
输出：
    无。
*/
void cb_func(client_data *user_data)
{
    // 超时后先把 fd 从 epoll 关注列表移除，再关闭 socket。
    epoll_ctl(Utils::u_epollfd, EPOLL_CTL_DEL, user_data->sockfd, 0);
    assert(user_data);
    close(user_data->sockfd);
    http_conn::m_user_count--;
}
