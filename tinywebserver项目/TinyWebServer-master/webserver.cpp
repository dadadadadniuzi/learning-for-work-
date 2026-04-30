#include "webserver.h"

/*
作用：
    构造服务器总控对象，提前分配“连接对象数组”和“连接对应的定时器数组”。
输入：
    无。
输出：
    无。
关键逻辑：
    这里只做资源准备，不真正创建监听 socket，也不启动事件循环。
*/
WebServer::WebServer()
{
    //http_conn类对象
    // 这里一次性创建 MAX_FD 个 http_conn 对象。
    // 后面每来一个连接 fd，都直接用 users[fd] 找到对应连接状态。
    users = new http_conn[MAX_FD];

    //root文件夹路径
    // getcwd 获取当前工作目录。
    char server_path[200];
    getcwd(server_path, 200);
    char root[6] = "/root";
    // 这里手动申请空间，后面用 strcpy + strcat 拼出网站根目录绝对路径。
    m_root = (char *)malloc(strlen(server_path) + strlen(root) + 1);
    strcpy(m_root, server_path);
    strcat(m_root, root);

    //定时器
    // 每个连接还要对应一份定时器数据，所以这里也按 MAX_FD 预分配。
    users_timer = new client_data[MAX_FD];
}

/*
作用：
    释放服务器运行期间申请的核心资源。
输入：
    无。
输出：
    无。
*/
WebServer::~WebServer()
{
    close(m_epollfd);
    close(m_listenfd);
    close(m_pipefd[1]);
    close(m_pipefd[0]);
    delete[] users;
    delete[] users_timer;
    delete m_pool;
}

/*
作用：
    保存服务器启动参数，供后续线程池、日志、数据库、监听 socket 初始化使用。
输入：
    port：监听端口。
    user / passWord / databaseName：MySQL 连接信息。
    log_write：日志模式，0 表示同步，1 表示异步。
    opt_linger：是否启用优雅关闭。
    trigmode：监听 fd 和连接 fd 的 LT/ET 组合模式。
    sql_num：数据库连接池大小。
    thread_num：线程池线程数量。
    close_log：是否关闭日志。
    actor_model：并发模型选择，0/1 对应不同处理风格。
输出：
    无。
*/
void WebServer::init(int port, string user, string passWord, string databaseName, int log_write, 
                     int opt_linger, int trigmode, int sql_num, int thread_num, int close_log, int actor_model)
{
    m_port = port;
    m_user = user;
    m_passWord = passWord;
    m_databaseName = databaseName;
    m_sql_num = sql_num;
    m_thread_num = thread_num;
    m_log_write = log_write;
    m_OPT_LINGER = opt_linger;
    m_TRIGMode = trigmode;
    m_close_log = close_log;
    m_actormodel = actor_model;
}

/*
作用：
    根据用户配置把监听 socket 和已连接 socket 的触发模式拆开保存。
输入：
    无，直接读取成员变量 m_TRIGMode。
输出：
    无。
*/
void WebServer::trig_mode()
{
    //LT + LT
    if (0 == m_TRIGMode)
    {
        m_LISTENTrigmode = 0;
        m_CONNTrigmode = 0;
    }
    //LT + ET
    else if (1 == m_TRIGMode)
    {
        m_LISTENTrigmode = 0;
        m_CONNTrigmode = 1;
    }
    //ET + LT
    else if (2 == m_TRIGMode)
    {
        m_LISTENTrigmode = 1;
        m_CONNTrigmode = 0;
    }
    //ET + ET
    else if (3 == m_TRIGMode)
    {
        m_LISTENTrigmode = 1;
        m_CONNTrigmode = 1;
    }
}

/*
作用：
    初始化日志系统。
输入：
    无，直接读取成员变量中的日志配置。
输出：
    无。
*/
void WebServer::log_write()
{
    if (0 == m_close_log)
    {
        //初始化日志
        if (1 == m_log_write)
            Log::get_instance()->init("./ServerLog", m_close_log, 2000, 800000, 800);
        else
            Log::get_instance()->init("./ServerLog", m_close_log, 2000, 800000, 0);
    }
}

/*
作用：
    初始化数据库连接池，并在启动阶段把 user 表里的账号密码加载到内存 map。
输入：
    无，直接使用成员变量中的数据库配置。
输出：
    无。
*/
void WebServer::sql_pool()
{
    //初始化数据库连接池
    m_connPool = connection_pool::GetInstance();
    m_connPool->init("localhost", m_user, m_passWord, m_databaseName, 3306, m_sql_num, m_close_log);

    //初始化数据库读取表
    users->initmysql_result(m_connPool);
}

/*
作用：
    创建线程池对象，让后续网络事件可以被分发给工作线程处理。
输入：
    无。
输出：
    无。
*/
void WebServer::thread_pool()
{
    //线程池
    m_pool = new threadpool<http_conn>(m_actormodel, m_connPool, m_thread_num);
}

void WebServer::eventListen()
{
    // 这个函数负责把服务器真正接入操作系统网络层：socket、bind、listen、再接入 epoll。
    //网络编程基础步骤
    // socket(PF_INET, SOCK_STREAM, 0)：
    // PF_INET 表示 IPv4，SOCK_STREAM 表示 TCP。
    // 返回值是新创建的 socket fd，失败返回 -1。
    m_listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(m_listenfd >= 0);

    //优雅关闭连接
    if (0 == m_OPT_LINGER)
    {
        struct linger tmp = {0, 1};
        setsockopt(m_listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
    }
    else if (1 == m_OPT_LINGER)
    {
        struct linger tmp = {1, 1};
        setsockopt(m_listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
    }

    int ret = 0;
    struct sockaddr_in address;
    // 先清空地址结构体，避免字段里残留旧数据。
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    // htonl / htons：把主机字节序转成网络字节序。
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(m_port);

    int flag = 1;
    // SO_REUSEADDR 允许端口复用，方便服务重启。
    setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    // bind 把 socket 绑定到指定 IP 和端口。
    ret = bind(m_listenfd, (struct sockaddr *)&address, sizeof(address));
    assert(ret >= 0);
    // listen 把普通 socket 变成监听 socket。
    ret = listen(m_listenfd, 5);
    assert(ret >= 0);

    utils.init(TIMESLOT);

    //epoll创建内核事件表
    epoll_event events[MAX_EVENT_NUMBER];
    // epoll 让主线程只关心“已经就绪的 fd”，而不是轮询每一个连接。
    m_epollfd = epoll_create(5);
    assert(m_epollfd != -1);

    utils.addfd(m_epollfd, m_listenfd, false, m_LISTENTrigmode);
    http_conn::m_epollfd = m_epollfd;

    // 通过这对管道把信号事件转换成普通 fd 事件，这样 epoll 就能统一处理。
    // socketpair 创建一对本地连通的 socket。
    // 这里专门用来把异步信号转成管道可读事件。
    ret = socketpair(PF_UNIX, SOCK_STREAM, 0, m_pipefd);
    assert(ret != -1);
    // 写端设成非阻塞，避免信号处理函数里写管道时卡住。
    utils.setnonblocking(m_pipefd[1]);
    utils.addfd(m_epollfd, m_pipefd[0], false, 0);

    utils.addsig(SIGPIPE, SIG_IGN);
    utils.addsig(SIGALRM, utils.sig_handler, false);
    utils.addsig(SIGTERM, utils.sig_handler, false);

    // alarm(TIMESLOT) 表示 TIMESLOT 秒后触发一次 SIGALRM。
    alarm(TIMESLOT);

    //工具类,信号和描述符基础操作
    Utils::u_pipefd = m_pipefd;
    Utils::u_epollfd = m_epollfd;
}

/*
作用：
    为一个新接入的客户端连接初始化 http_conn 对象，并创建对应的超时定时器。
输入：
    connfd：新连接的文件描述符。
    client_address：客户端地址。
输出：
    无。
*/
void WebServer::timer(int connfd, struct sockaddr_in client_address)
{
    users[connfd].init(connfd, client_address, m_root, m_CONNTrigmode, m_close_log, m_user, m_passWord, m_databaseName);
    // 每个新连接一建立就绑定一个定时器，后续只要连接活跃就会延后超时时间。

    //初始化client_data数据
    //创建定时器，设置回调函数和超时时间，绑定用户数据，将定时器添加到链表中
    users_timer[connfd].address = client_address;
    users_timer[connfd].sockfd = connfd;
    // 每个连接创建一个独立定时器节点，后续超时扫描就是处理这些节点。
    util_timer *timer = new util_timer;
    timer->user_data = &users_timer[connfd];
    timer->cb_func = cb_func;
    // time(NULL) 返回当前时间戳，expire 表示“什么时候算超时”。
    time_t cur = time(NULL);
    timer->expire = cur + 3 * TIMESLOT;
    users_timer[connfd].timer = timer;
    utils.m_timer_lst.add_timer(timer);
}

//若有数据传输，则将定时器往后延迟3个单位
//并对新的定时器在链表上的位置进行调整
/*
作用：
    连接上有读写活动时，延长它的超时时间，避免活跃连接被误关闭。
输入：
    timer：该连接对应的定时器节点。
输出：
    无。
*/
void WebServer::adjust_timer(util_timer *timer)
{
    time_t cur = time(NULL);
    timer->expire = cur + 3 * TIMESLOT;
    utils.m_timer_lst.adjust_timer(timer);

    LOG_INFO("%s", "adjust timer once");
}

/*
作用：
    处理超时或异常连接，执行关闭逻辑并从定时器链表中删除节点。
输入：
    timer：待删除的定时器。
    sockfd：对应连接的文件描述符。
输出：
    无。
*/
void WebServer::deal_timer(util_timer *timer, int sockfd)
{
    timer->cb_func(&users_timer[sockfd]);
    if (timer)
    {
        utils.m_timer_lst.del_timer(timer);
    }

    LOG_INFO("close fd %d", users_timer[sockfd].sockfd);
}

/*
作用：
    处理新的客户端连接。
输入：
    无，内部从监听 socket 调用 accept。
输出：
    true：成功接收到了连接。
    false：本轮没有成功接收到连接，或 ET 模式下已经把 accept 队列取空。
*/
bool WebServer::dealclientdata()
{
    struct sockaddr_in client_address;
    socklen_t client_addrlength = sizeof(client_address);
    if (0 == m_LISTENTrigmode)
    {
        // accept 从监听队列中取出一个已完成三次握手的客户端连接。
        int connfd = accept(m_listenfd, (struct sockaddr *)&client_address, &client_addrlength);
        if (connfd < 0)
        {
            LOG_ERROR("%s:errno is:%d", "accept error", errno);
            return false;
        }
        if (http_conn::m_user_count >= MAX_FD)
        {
            utils.show_error(connfd, "Internal server busy");
            LOG_ERROR("%s", "Internal server busy");
            return false;
        }
        timer(connfd, client_address);
    }

    else
    {
        // ET 模式下必须一次性把 accept 队列取空，否则可能漏掉后续新连接通知。
        while (1)
        {
            // ET 模式下必须循环 accept，直到队列取空。
            int connfd = accept(m_listenfd, (struct sockaddr *)&client_address, &client_addrlength);
            if (connfd < 0)
            {
                LOG_ERROR("%s:errno is:%d", "accept error", errno);
                break;
            }
            if (http_conn::m_user_count >= MAX_FD)
            {
                utils.show_error(connfd, "Internal server busy");
                LOG_ERROR("%s", "Internal server busy");
                break;
            }
            timer(connfd, client_address);
        }
        return false;
    }
    return true;
}

/*
作用：
    读取信号管道中的字节，把异步信号转成主循环里的普通标志位。
输入：
    timeout：输出参数，收到 SIGALRM 时被置为 true。
    stop_server：输出参数，收到 SIGTERM 时被置为 true。
输出：
    true：读取成功。
    false：读取失败或管道已关闭。
*/
bool WebServer::dealwithsignal(bool &timeout, bool &stop_server)
{
    int ret = 0;
    int sig;
    char signals[1024];
    // recv 从管道读端把信号编号取出来。
    ret = recv(m_pipefd[0], signals, sizeof(signals), 0);
    if (ret == -1)
    {
        return false;
    }
    else if (ret == 0)
    {
        return false;
    }
    else
    {
        for (int i = 0; i < ret; ++i)
        {
            switch (signals[i])
            {
            case SIGALRM:
            {
                timeout = true;
                break;
            }
            case SIGTERM:
            {
                stop_server = true;
                break;
            }
            }
        }
    }
    return true;
}

void WebServer::dealwithread(int sockfd)
{
    util_timer *timer = users_timer[sockfd].timer;

    //reactor
    if (1 == m_actormodel)
    {
        if (timer)
        {
            adjust_timer(timer);
        }

        //若监测到读事件，将该事件放入请求队列
        // Reactor 路径：主线程只负责分发，可读事件真正的 read 由工作线程执行。
        // append(..., 0) 表示把“读任务”投递给线程池。
        m_pool->append(users + sockfd, 0);

        while (true)
        {
            if (1 == users[sockfd].improv)
            {
                if (1 == users[sockfd].timer_flag)
                {
                    deal_timer(timer, sockfd);
                    users[sockfd].timer_flag = 0;
                }
                users[sockfd].improv = 0;
                break;
            }
        }
    }
    else
    {
        //proactor
        // Proactor 风格路径：主线程先完成读，工作线程主要处理后续业务逻辑。
        // Proactor 风格：主线程先完成读，再把处理逻辑交给工作线程。
        // Proactor 路径下，主线程先把 socket 数据读到连接对象自己的缓冲区里。
        if (users[sockfd].read_once())
        {
            LOG_INFO("deal with the client(%s)", inet_ntoa(users[sockfd].get_address()->sin_addr));

            //若监测到读事件，将该事件放入请求队列
            // 然后把“后续解析和业务处理”投递给工作线程。
            m_pool->append_p(users + sockfd);

            if (timer)
            {
                adjust_timer(timer);
            }
        }
        else
        {
            // 主线程读失败，通常说明连接异常或对端关闭，直接清理。
            deal_timer(timer, sockfd);
        }
    }
}

void WebServer::dealwithwrite(int sockfd)
{
    util_timer *timer = users_timer[sockfd].timer;
    //reactor
    if (1 == m_actormodel)
    {
        if (timer)
        {
            adjust_timer(timer);
        }

        // Reactor 路径下的写事件：真正的 write 由工作线程执行。
        // append(..., 1) 表示把“写任务”投递给线程池。
        // 这里只是把“写任务”交给线程池，真正 send/writev 在工作线程里执行。
        m_pool->append(users + sockfd, 1);

        while (true)
        {
            // 主线程忙等这个标志，直到工作线程把这一轮写任务处理完。
            if (1 == users[sockfd].improv)
            {
                if (1 == users[sockfd].timer_flag)
                {
                    deal_timer(timer, sockfd);
                    users[sockfd].timer_flag = 0;
                }
                users[sockfd].improv = 0;
                break;
            }
        }
    }
    else
    {
        //proactor
        // Proactor 风格路径下：主线程直接执行 write。
        // Proactor 风格下，可写事件直接由主线程执行 write。
        // Proactor 路径下，主线程自己完成写回客户端。
        if (users[sockfd].write())
        {
            LOG_INFO("send data to the client(%s)", inet_ntoa(users[sockfd].get_address()->sin_addr));

            if (timer)
            {
                adjust_timer(timer);
            }
        }
        else
        {
            deal_timer(timer, sockfd);
        }
    }
}

void WebServer::eventLoop()
{
    bool timeout = false;
    bool stop_server = false;
    // 这里是整个服务器的交通枢纽：新连接、读写事件、信号事件、超时事件都在这里汇合。

    while (!stop_server)
    {
        // epoll_wait 阻塞等待内核返回“已经就绪”的事件列表。
        // -1 表示无限等待，直到有事件发生。
        // epoll_wait 返回“这一次已经就绪的所有事件”数量。
        int number = epoll_wait(m_epollfd, events, MAX_EVENT_NUMBER, -1);
        if (number < 0 && errno != EINTR)
        {
            LOG_ERROR("%s", "epoll failure");
            break;
        }

        for (int i = 0; i < number; i++)
        {
            int sockfd = events[i].data.fd;

            //处理新到的客户连接
            if (sockfd == m_listenfd)
            {
                // 监听 fd 可读，说明有新的客户端正在完成三次握手，应该 accept。
                bool flag = dealclientdata();
                if (false == flag)
                    continue;
            }
            else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            {
                //服务器端关闭连接，移除对应的定时器
                util_timer *timer = users_timer[sockfd].timer;
                // 连接异常或被对端关闭，直接走清理逻辑。
                deal_timer(timer, sockfd);
            }
            //处理信号
            else if ((sockfd == m_pipefd[0]) && (events[i].events & EPOLLIN))
            {
                bool flag = dealwithsignal(timeout, stop_server);
                if (false == flag)
                    LOG_ERROR("%s", "dealclientdata failure");
            }
            //处理客户连接上接收到的数据
            else if (events[i].events & EPOLLIN)
            {
                // 某个已连接客户端有数据可读，进入读事件处理。
                dealwithread(sockfd);
            }
            else if (events[i].events & EPOLLOUT)
            {
                // 某个连接现在可写，说明可以继续回发响应。
                dealwithwrite(sockfd);
            }
        }
        if (timeout)
        {
            // timeout 由 SIGALRM 转换而来，表示该做一轮超时连接扫描了。
            utils.timer_handler();

            LOG_INFO("%s", "timer tick");

            timeout = false;
        }
    }
}
