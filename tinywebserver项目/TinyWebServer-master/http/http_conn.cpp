#include "http_conn.h"

#include <mysql/mysql.h>
#include <fstream>

//定义http响应的一些状态信息
const char *ok_200_title = "OK";
const char *error_400_title = "Bad Request";
const char *error_400_form = "Your request has bad syntax or is inherently impossible to staisfy.\n";
const char *error_403_title = "Forbidden";
const char *error_403_form = "You do not have permission to get file form this server.\n";
const char *error_404_title = "Not Found";
const char *error_404_form = "The requested file was not found on this server.\n";
const char *error_500_title = "Internal Error";
const char *error_500_form = "There was an unusual problem serving the request file.\n";

locker m_lock;
map<string, string> users;

/*
作用：
    服务器启动时预加载数据库中的用户名和密码，减少每次登录都查库的开销。
输入：
    connPool：数据库连接池。
输出：
    无。
*/
void http_conn::initmysql_result(connection_pool *connPool)
{
    // 启动时先把数据库里的用户加载进内存 map，后续登录校验优先走内存。
    //先从连接池中取一个连接
    // MYSQL* 是 MySQL C API 中的连接句柄类型。
    MYSQL *mysql = NULL;
    connectionRAII mysqlcon(&mysql, connPool);

    //在user表中检索username，passwd数据，浏览器端输入
    // mysql_query 发送 SQL 语句到数据库执行。
    if (mysql_query(mysql, "SELECT username,passwd FROM user"))
    {
        LOG_ERROR("SELECT error:%s\n", mysql_error(mysql));
    }

    //从表中检索完整的结果集
    // mysql_store_result 把查询结果集完整拉回客户端内存。
    MYSQL_RES *result = mysql_store_result(mysql);

    //返回结果集中的列数
    int num_fields = mysql_num_fields(result);

    //返回所有字段结构的数组
    MYSQL_FIELD *fields = mysql_fetch_fields(result);

    //从结果集中获取下一行，将对应的用户名和密码，存入map中
    // mysql_fetch_row 每次取一行数据，直到结果集结束返回 NULL。
    while (MYSQL_ROW row = mysql_fetch_row(result))
    {
        string temp1(row[0]);
        string temp2(row[1]);
        users[temp1] = temp2;
    }
}

//对文件描述符设置非阻塞
int setnonblocking(int fd)
{
    // fcntl(F_GETFL) 读取当前 fd 的状态标志。
    int old_option = fcntl(fd, F_GETFL);
    // O_NONBLOCK 表示把 fd 设置成非阻塞。
    int new_option = old_option | O_NONBLOCK;
    // fcntl(F_SETFL) 把新的标志重新写回。
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

//将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
void addfd(int epollfd, int fd, bool one_shot, int TRIGMode)
{
    epoll_event event;
    // data.fd 记录这个事件对应的是哪个文件描述符。
    event.data.fd = fd;

    if (1 == TRIGMode)
        event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    else
        event.events = EPOLLIN | EPOLLRDHUP;

    if (one_shot)
        event.events |= EPOLLONESHOT;
    // epoll_ctl(..., EPOLL_CTL_ADD, ...) 表示把 fd 注册进 epoll。
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

//从内核时间表删除描述符
void removefd(int epollfd, int fd)
{
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
}

//将事件重置为EPOLLONESHOT
void modfd(int epollfd, int fd, int ev, int TRIGMode)
{
    epoll_event event;
    event.data.fd = fd;

    if (1 == TRIGMode)
        event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    else
        event.events = ev | EPOLLONESHOT | EPOLLRDHUP;

    // EPOLL_CTL_MOD 表示修改 fd 已经关注的事件集合。
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

int http_conn::m_user_count = 0;
int http_conn::m_epollfd = -1;

//关闭连接，关闭一个连接，客户总量减一
/*
作用：
    关闭当前 HTTP 连接，并把它从 epoll 监控集合中移除。
输入：
    real_close：是否真正关闭底层 socket。
输出：
    无。
*/
void http_conn::close_conn(bool real_close)
{
    if (real_close && (m_sockfd != -1))
    {
        printf("close %d\n", m_sockfd);
        removefd(m_epollfd, m_sockfd);
        m_sockfd = -1;
        m_user_count--;
    }
}

//初始化连接,外部调用初始化套接字地址
/*
作用：
    用新接入的 socket 初始化一个 http_conn 对象。
输入：
    sockfd：客户端连接 fd。
    addr：客户端地址。
    root：网站根目录。
    TRIGMode：连接 fd 的 LT/ET 模式。
    close_log：日志开关。
    user / passwd / sqlname：数据库认证信息。
输出：
    无。
*/
void http_conn::init(int sockfd, const sockaddr_in &addr, char *root, int TRIGMode,
                     int close_log, string user, string passwd, string sqlname)
{
    m_sockfd = sockfd;
    m_address = addr;

    // one_shot=true 可以避免同一个连接被多个线程并发处理。
    addfd(m_epollfd, sockfd, true, m_TRIGMode);
    m_user_count++;

    //当浏览器出现连接重置时，可能是网站根目录出错或http响应格式出错或者访问的文件中内容完全为空
    doc_root = root;
    m_TRIGMode = TRIGMode;
    m_close_log = close_log;

    // c_str() 把 C++ string 转成 const char*，strcpy 再拷贝到类成员字符数组里。
    strcpy(sql_user, user.c_str());
    strcpy(sql_passwd, passwd.c_str());
    strcpy(sql_name, sqlname.c_str());

    init();
}

//初始化新接受的连接
//check_state默认为分析请求行状态
/*
作用：
    重置连接对象内部状态。
输入：
    无。
输出：
    无。
说明：
    keep-alive 场景下一次响应结束后，还会再次调用它，为下一轮请求清空状态。
*/
void http_conn::init()
{
    mysql = NULL;
    bytes_to_send = 0;
    bytes_have_send = 0;
    // 每次初始化都从“解析请求行”开始。
    // 因为一个完整 HTTP 请求最先出现的一定是请求行。
    m_check_state = CHECK_STATE_REQUESTLINE;
    m_linger = false;
    m_method = GET;
    m_url = 0;
    m_version = 0;
    m_content_length = 0;
    m_host = 0;
    // m_start_line：当前这一行在读缓冲区里的起始下标。
    m_start_line = 0;
    // m_checked_idx：从状态机已经检查到读缓冲区的哪个位置。
    m_checked_idx = 0;
    // m_read_idx：已经从 socket 读入了多少字节。
    m_read_idx = 0;
    m_write_idx = 0;
    cgi = 0;
    m_state = 0;
    timer_flag = 0;
    improv = 0;

    // 清空缓冲区，避免 keep-alive 场景下上一个请求的残留数据干扰下一轮解析。
    memset(m_read_buf, '\0', READ_BUFFER_SIZE);
    memset(m_write_buf, '\0', WRITE_BUFFER_SIZE);
    memset(m_real_file, '\0', FILENAME_LEN);
}

//从状态机，用于分析出一行内容
//返回值为行的读取状态，有LINE_OK,LINE_BAD,LINE_OPEN
http_conn::LINE_STATUS http_conn::parse_line()
{
    // 从状态机只解决一个问题：当前缓冲区里是否已经拿到一整行 HTTP 文本。
    // 这个函数的任务是：
    // 在当前缓冲区中寻找一整行 HTTP 文本的结束位置。
    // 一整行的合法结束标志通常是 "\r\n"。
    // 找到后会把 "\r\n" 改写成 "\0\0"，方便后续当作 C 字符串处理。
    char temp;
    // 注意这里不是每次都从 0 开始扫，而是从上次已经检查到的位置继续往后扫。
    for (; m_checked_idx < m_read_idx; ++m_checked_idx)
    {
        temp = m_read_buf[m_checked_idx];
        // 扫到 '\r' 时，说明这一行可能到结尾了。
        if (temp == '\r')
        {
            // 如果 '\r' 已经是本轮缓冲区最后一个字节，
            // 说明 '\n' 还没到达，这一行暂时还不完整。
            if ((m_checked_idx + 1) == m_read_idx)
                return LINE_OPEN;
            // 如果后一个字节正好是 '\n'，说明这一行完整结束。
            else if (m_read_buf[m_checked_idx + 1] == '\n')
            {
                // 把 "\r\n" 原地改成 "\0\0"，
                // 这样这一行就能直接当 C 字符串交给后面的解析函数。
                m_read_buf[m_checked_idx++] = '\0';
                m_read_buf[m_checked_idx++] = '\0';
                return LINE_OK;
            }
            // 有 '\r' 但后面不是 '\n'，格式非法。
            return LINE_BAD;
        }
        // 如果直接遇到 '\n'，还要检查前一位是不是 '\r'。
        else if (temp == '\n')
        {
            if (m_checked_idx > 1 && m_read_buf[m_checked_idx - 1] == '\r')
            {
                // 这是另一种命中行尾的路径：
                // 当前是 '\n'，前一个是 '\r'，同样把这一行原地截断出来。
                m_read_buf[m_checked_idx - 1] = '\0';
                m_read_buf[m_checked_idx++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }
    // 扫描到当前有效数据末尾也没找到完整行，
    // 说明还要继续从 socket 收数据。
    return LINE_OPEN;
}

//循环读取客户数据，直到无数据可读或对方关闭连接
//非阻塞ET工作模式下，需要一次性将数据读完
bool http_conn::read_once()
{
    // 如果缓冲区已经满了，再读就会越界。
    if (m_read_idx >= READ_BUFFER_SIZE)
    {
        return false;
    }
    // bytes_read 表示本次 recv 实际读取到的字节数。
    int bytes_read = 0;

    //LT读取数据
    // LT 模式下不要求一次事件通知就把 socket 中的数据全部读空。
    if (0 == m_TRIGMode)
    {
        // recv 把新数据追加到读缓冲区尾部，避免覆盖之前没解析完的数据。
        // LT 模式下，本轮先读一次。
        // 新数据接在旧数据后面，避免覆盖上一次未处理完的半包数据。
        bytes_read = recv(m_sockfd, m_read_buf + m_read_idx, READ_BUFFER_SIZE - m_read_idx, 0);
        // 推进“当前总共已读到哪里”的尾指针。
        m_read_idx += bytes_read;

        if (bytes_read <= 0)
        {
            return false;
        }

        return true;
    }
    //ET读数据
    // ET 模式下必须一直读到 EAGAIN，否则剩余数据可能不会再次触发读事件。
    else
    {
        // ET 模式下必须尽量把内核缓冲区读空。
        while (true)
        {
            bytes_read = recv(m_sockfd, m_read_buf + m_read_idx, READ_BUFFER_SIZE - m_read_idx, 0);
            if (bytes_read == -1)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    // 非阻塞模式下读到这里表示“暂时没有更多数据”，不是错误。
                    break;
                return false;
            }
            // recv 返回 0 说明对端关闭连接。
            else if (bytes_read == 0)
            {
                return false;
            }
            // 每成功读到一段，都把有效数据尾部向后推进。
            m_read_idx += bytes_read;
        }
        return true;
    }
}

//解析http请求行，获得请求方法，目标url及http版本号
http_conn::HTTP_CODE http_conn::parse_request_line(char *text)
{
    // 主状态机从这里开始，先拆出 method、URL 和 HTTP version。
    // strpbrk 用来在请求行里找到 method 和 URL 之间的第一个空格/制表符。
    // 先找 method 和 URL 之间的第一个空格/制表符。
    // 例如 "GET /index.html HTTP/1.1" 里，会定位到 GET 后面的那个空格。
    m_url = strpbrk(text, " \t");
    if (!m_url)
    {
        return BAD_REQUEST;
    }
    // 把分隔空格改成 '\0'，等于在原字符串上原地切段。
    // 把 method 后面的分隔空格原地改成 '\0'，
    // 这样 text 这段就变成了独立的 method 字符串。
    // m_url++ 之后，m_url 指向分隔符后面的下一个字符。
    *m_url++ = '\0';
    // 现在 text 指向的方法名就是一个完整字符串，例如 "GET" 或 "POST"。
    char *method = text;
    if (strcasecmp(method, "GET") == 0)
        m_method = GET;
    else if (strcasecmp(method, "POST") == 0)
    {
        m_method = POST;
        // 这个项目把 POST 请求和表单/CGI 风格处理关联起来。
        cgi = 1;
    }
    else
        // 不是项目支持的方法，直接判为坏请求。
        return BAD_REQUEST;
    // strspn 跳过开头连续的空格和制表符。
    // strspn(m_url, " \t") 会返回“从 m_url 开始连续有多少个空格或制表符”。
    // 这里的作用是跳过 method 后面和 URL 前面多余的空白字符，
    // 让 m_url 真正指向 URL 的第一个有效字符。
    m_url += strspn(m_url, " \t");
    // 现在 m_url 指向 URL 开头，再次用 strpbrk 查找后面的空格/制表符，
    // 就能找到 URL 和 HTTP 版本号之间的分隔位置。
    m_version = strpbrk(m_url, " \t");
    // 没找到 URL 和版本号之间的分隔空白，说明请求行格式不完整。
    if (!m_version)
        return BAD_REQUEST;
    // 再切一次，把 URL 和 HTTP 版本分开。
    // 把 URL 后面的空格原地改成 '\0'，
    // 这样 m_url 这段内容就单独变成了一个完整字符串。
    // m_version++ 的副作用是：先把当前位置置为 '\0'，再让 m_version 指向下一个字符。
    *m_version++ = '\0';
    // 有些请求行里 URL 和版本号之间可能不止一个空格，
    // 所以这里继续跳过所有连续空白，让 m_version 指向真正的版本号文本。
    m_version += strspn(m_version, " \t");
    // strcasecmp 是“不区分大小写的字符串比较”。
    // 返回 0 表示相等；不等于 0 表示版本不是 HTTP/1.1。
    // 这个项目只接受 HTTP/1.1，请求版本不匹配就直接判为坏请求。
    if (strcasecmp(m_version, "HTTP/1.1") != 0)
        return BAD_REQUEST;
    // 有些客户端发来的 URL 不是相对路径，而是完整形式：
    // GET http://example.com/index.html HTTP/1.1
    // strncasecmp(m_url, "http://", 7) == 0
    // 表示“忽略大小写，只比较前 7 个字符，并且它们刚好是 http://”。
    if (strncasecmp(m_url, "http://", 7) == 0)
    {
        // m_url += 7 之后，m_url 不再指向 "http://",
        // 而是改为指向后面的主机名，例如 "example.com/index.html"。
        m_url += 7;
        // strchr(m_url, '/') 会从当前位置开始查找第一个 '/'。
        // 对于 "example.com/index.html"，找到的就是 "/index.html" 开头的位置。
        // 这样就把“协议 + 域名”部分跳过去，只保留服务器真正关心的资源路径部分。
        m_url = strchr(m_url, '/');
    }

    // https:// 的完整 URL 也做同样处理。
    // 这里只是把 URL 规范化成路径，不代表这个项目真的完成了 HTTPS 握手。
    if (strncasecmp(m_url, "https://", 8) == 0)
    {
        // 跳过 "https://" 这 8 个字符，让指针移动到主机名开头。
        m_url += 8;
        // 再找到后面的第一个 '/'，把 m_url 调整为真正的资源路径。
        m_url = strchr(m_url, '/');
    }

    // 走到这里时，m_url 应该已经是 "/xxx/yyy" 这种路径格式。
    // 如果 m_url 为空，或者第一个字符不是 '/'，说明请求行格式仍然不合法。
    if (!m_url || m_url[0] != '/')
        return BAD_REQUEST;
    //当url为/时，显示判断界面
    // 只有 "/" 时，项目约定默认跳到判定首页。
    if (strlen(m_url) == 1)
        strcat(m_url, "judge.html");
    // 请求行解析完成后，主状态机切到“继续解析请求头”阶段。
    m_check_state = CHECK_STATE_HEADER;
    return NO_REQUEST;
}

//解析http请求的一个头部信息
http_conn::HTTP_CODE http_conn::parse_headers(char *text)
{
    // 如果这一行是空串，说明已经读到了“请求头结束的空行”。
    // 请求头和请求体之间有一行空行。
    // parse_line 已经把每一行切成字符串，所以这里看到空串就说明头部结束了。
    if (text[0] == '\0')
    {
        // 如果之前解析到 Content-Length，说明后面还跟着请求体。
        if (m_content_length != 0)
        {
            // 请求头结束，但因为有 Content-Length，后面还要继续解析请求体。
            m_check_state = CHECK_STATE_CONTENT;
            return NO_REQUEST;
        }
        // 没有请求体，说明请求到这里已经完整结束。
        return GET_REQUEST;
    }
    else if (strncasecmp(text, "Connection:", 11) == 0)
    {
        // 跳过 "Connection:" 这个前缀，开始读它后面的值。
        text += 11;
        // 跳过冒号后可能出现的空格或制表符，让 text 指向真正的字段值。
        text += strspn(text, " \t");
        if (strcasecmp(text, "keep-alive") == 0)
        {
            // 记录 keep-alive，后面 write 完响应时决定是否复用连接。
            m_linger = true;
        }
    }
    else if (strncasecmp(text, "Content-length:", 15) == 0)
    {
        // 跳过 "Content-length:" 这个前缀。
        text += 15;
        // 跳过字段值前面的空白字符。
        text += strspn(text, " \t");
        // atol 把字符串形式的内容长度转成 long。
        // 解析请求体长度，后面 parse_content 会按这个长度判断 body 是否收完整。
        m_content_length = atol(text);
    }
    else if (strncasecmp(text, "Host:", 5) == 0)
    {
        // 跳过 "Host:" 这个前缀。
        text += 5;
        // 跳过字段值前面的空白字符。
        text += strspn(text, " \t");
        // 直接让 m_host 指向当前这行里 Host 字段值的位置，不额外复制。
        m_host = text;
    }
    else
    {
        // 对这个简化项目不关心的头字段，不报错，只打印日志。
        LOG_INFO("oop!unknow header: %s", text);
    }
    // 当前这一行请求头处理完了，但整个请求可能还没结束，继续往下解析。
    return NO_REQUEST;
}

//判断http请求是否被完整读入
/*
作用：
    判断请求体是否已经完整到达。
输入：
    text：请求体起始位置。
输出：
    GET_REQUEST：请求体已经收完整。
    NO_REQUEST：请求体还没收全，需要继续读 socket。
*/
http_conn::HTTP_CODE http_conn::parse_content(char *text)
{
    // 当前已经读到的总字节数足够覆盖整个请求体，才说明 body 收完整了。
    // 判断依据是：
    // 当前已读总字节数，是否已经覆盖“请求体起始位置 + Content-Length”。
    if (m_read_idx >= (m_content_length + m_checked_idx))
    {
        // 在原缓冲区里把请求体截断成字符串，方便后面直接解析表单内容。
        text[m_content_length] = '\0';
        //POST请求中最后为输入的用户名和密码
        // m_string 直接指向请求体字符串，后面登录/注册逻辑会从这里解析表单内容。
        m_string = text;
        return GET_REQUEST;
    }
    return NO_REQUEST;
}

http_conn::HTTP_CODE http_conn::process_read()
{
    // process_read 是 HTTP 解析总调度：把“按行切分”和“请求行/请求头/请求体解析”串在一起。
    // process_read 是整个 HTTP 解析阶段的总控函数：
    // 先切行，再根据主状态机阶段把这一行交给不同的解析函数。
    // 从状态机返回“按行切分”的结果。
    LINE_STATUS line_status = LINE_OK;
    // ret 用来接收每个阶段解析后的结果码。
    HTTP_CODE ret = NO_REQUEST;
    // text 始终指向“当前这一轮要解析的那一段字符串”的起始位置。
    char *text = 0;

    // 条件解释：
    // 这是整个 HTTP 状态机最核心的一行：
    // 1. 如果已经进入请求体阶段，就不再依赖 parse_line，因为 body 不一定按行组织。
    // 2. 如果还在请求行/请求头阶段，就必须先通过 parse_line 切出一整行。
    while ((m_check_state == CHECK_STATE_CONTENT && line_status == LINE_OK) || ((line_status = parse_line()) == LINE_OK))
    {
        // 取出当前这一行的起始地址。
        // 取出当前这一行在读缓冲区里的起始地址。
        text = get_line();
  
        // parse_line 结束后，m_checked_idx 已经推进到下一行开头，
        // 所以这里同步更新 m_start_line，供下一轮 get_line() 使用。
        m_start_line = m_checked_idx;
        // 调试时可以看到当前状态机正在处理哪一行文本。
        LOG_INFO("%s", text);
        switch (m_check_state)
        {
        case CHECK_STATE_REQUESTLINE:
        {

            // 第 1 阶段：解析请求行，目标是拆出 method / URL / version。
            ret = parse_request_line(text);
            if (ret == BAD_REQUEST)
                return BAD_REQUEST;
            break;
        }
        case CHECK_STATE_HEADER:
        {

            // 第 2 阶段：逐行解析请求头字段。
            ret = parse_headers(text);
            if (ret == BAD_REQUEST)
                return BAD_REQUEST;
            else if (ret == GET_REQUEST)
            {
                // 请求头已经处理完，并且请求整体完整，可以进入业务处理。
                // 请求已经完整，可以正式进入资源定位或登录/注册业务逻辑。
                return do_request();
            }
            break;
        }
        case CHECK_STATE_CONTENT:
        {
            // 第 3 阶段：处理请求体。
            // 第 3 阶段：处理请求体。
            ret = parse_content(text);
            if (ret == GET_REQUEST)
                // 请求体已经收完整，正式进入业务处理。
                // 请求体已经收完整，正式进入业务处理。
                return do_request();
            // body 还没收完整，退出本轮解析，等待下次继续读。
            // 请求体还没收完整，主动跳出当前 while，等下一轮继续收数据。
            line_status = LINE_OPEN;
            break;
        }
        default:
            return INTERNAL_ERROR;
        }
    }
    // 当前请求还没有完整收齐，通知上层继续 read socket。
    // 走到这里说明当前缓冲区里的数据还不够凑成一个完整请求。
    // 上层看到 NO_REQUEST 后，会继续监听读事件等待更多数据。
    return NO_REQUEST;
}

/*
作用：
    根据 URL 和业务规则执行真正的请求处理。
输入：
    无，直接使用前面解析出来的 m_url、m_method、m_string 等成员变量。
输出：
    FILE_REQUEST / NO_RESOURCE / FORBIDDEN_REQUEST / BAD_REQUEST 等结果码。
说明：
    这里既处理静态资源访问，也处理登录/注册这类简单 CGI 逻辑。
*/
http_conn::HTTP_CODE http_conn::do_request()
{
    // 真正的业务入口在这里：要么把 URL 映射成静态文件，要么进入登录注册逻辑。
    // 先把网站根目录放进目标路径缓冲区，后面再拼接具体 URL 文件名。
    // 先把网站根目录写进最终路径缓冲区，后面再拼接具体页面或资源路径。
    strcpy(m_real_file, doc_root);
    int len = strlen(doc_root);
    //printf("m_url:%s\n", m_url);
    // strrchr 从右往左找最后一个 '/'，这里用于判断访问的是哪类业务页面。
    // 找到 URL 里最后一个 '/'，这个项目会根据最后一个字符做页面/业务分流。
    const char *p = strrchr(m_url, '/');

    //处理cgi
    // 只有 POST 并且 URL 命中作者约定的业务标志位时，
    // 才进入这里的登录/注册逻辑。
    if (cgi == 1 && (*(p + 1) == '2' || *(p + 1) == '3'))
    {

        //根据标志判断是登录检测还是注册检测
        char flag = m_url[1];

        // 拼接临时页面路径。
        char *m_url_real = (char *)malloc(sizeof(char) * 200);
        strcpy(m_url_real, "/");
        strcat(m_url_real, m_url + 2);
        strncpy(m_real_file + len, m_url_real, FILENAME_LEN - len - 1);
        free(m_url_real);

        //将用户名和密码提取出来
        //user=123&passwd=123
        // 下面开始手工解析 POST 表单：
        // user=xxx&passwd=yyy
        char name[100], password[100];
        int i;
        // 手工解析 POST 表单字符串：user=xxx&passwd=yyy
        for (i = 5; m_string[i] != '&'; ++i)
            name[i - 5] = m_string[i];
        name[i - 5] = '\0';

        int j = 0;
        for (i = i + 10; m_string[i] != '\0'; ++i, ++j)
            password[j] = m_string[i];
        password[j] = '\0';

        // '3' 表示注册分支。
        if (*(p + 1) == '3')
        {
            //如果是注册，先检测数据库中是否有重名的
            //没有重名的，进行增加数据
            char *sql_insert = (char *)malloc(sizeof(char) * 200);
            strcpy(sql_insert, "INSERT INTO user(username, passwd) VALUES(");
            strcat(sql_insert, "'");
            strcat(sql_insert, name);
            strcat(sql_insert, "', '");
            strcat(sql_insert, password);
            strcat(sql_insert, "')");

            // 用户名不存在时才允许注册。
            if (users.find(name) == users.end())
            {
                m_lock.lock();
                // 执行注册 SQL，把新用户写入数据库。
                int res = mysql_query(mysql, sql_insert);
                users.insert(pair<string, string>(name, password));
                m_lock.unlock();

                if (!res)
                    strcpy(m_url, "/log.html");
                else
                    strcpy(m_url, "/registerError.html");
            }
            else
                strcpy(m_url, "/registerError.html");
        }
        //如果是登录，直接判断
        //若浏览器端输入的用户名和密码在表中可以查找到，返回1，否则返回0
        // '2' 表示登录分支。
        else if (*(p + 1) == '2')
        {
            if (users.find(name) != users.end() && users[name] == password)
                strcpy(m_url, "/welcome.html");
            else
                strcpy(m_url, "/logError.html");
        }
    }

    if (*(p + 1) == '0')
    {
        char *m_url_real = (char *)malloc(sizeof(char) * 200);
        strcpy(m_url_real, "/register.html");
        strncpy(m_real_file + len, m_url_real, strlen(m_url_real));

        free(m_url_real);
    }
    else if (*(p + 1) == '1')
    {
        char *m_url_real = (char *)malloc(sizeof(char) * 200);
        strcpy(m_url_real, "/log.html");
        strncpy(m_real_file + len, m_url_real, strlen(m_url_real));

        free(m_url_real);
    }
    else if (*(p + 1) == '5')
    {
        char *m_url_real = (char *)malloc(sizeof(char) * 200);
        strcpy(m_url_real, "/picture.html");
        strncpy(m_real_file + len, m_url_real, strlen(m_url_real));

        free(m_url_real);
    }
    else if (*(p + 1) == '6')
    {
        char *m_url_real = (char *)malloc(sizeof(char) * 200);
        strcpy(m_url_real, "/video.html");
        strncpy(m_real_file + len, m_url_real, strlen(m_url_real));

        free(m_url_real);
    }
    else if (*(p + 1) == '7')
    {
        char *m_url_real = (char *)malloc(sizeof(char) * 200);
        strcpy(m_url_real, "/fans.html");
        strncpy(m_real_file + len, m_url_real, strlen(m_url_real));

        free(m_url_real);
    }
    else
        strncpy(m_real_file + len, m_url, FILENAME_LEN - len - 1);

    // stat 既可以判断文件是否存在，也能拿到权限、大小、类型等元数据。
    // stat 用来检查文件是否存在，并获取它的权限、大小、类型等信息。
    if (stat(m_real_file, &m_file_stat) < 0)
        return NO_RESOURCE;

    if (!(m_file_stat.st_mode & S_IROTH))
        return FORBIDDEN_REQUEST;

    if (S_ISDIR(m_file_stat.st_mode))
        return BAD_REQUEST;

    // open 以只读方式打开目标文件。
    // 通过 open 以只读方式打开目标文件。
    int fd = open(m_real_file, O_RDONLY);
    // mmap 把文件映射到内存，后面就能直接配合 writev 发送。
    // mmap 把文件映射到内存，后面 writev 可以直接发送这段映射内存。
    m_file_address = (char *)mmap(0, m_file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    return FILE_REQUEST;
}
/*
作用：
    解除静态文件的内存映射。
输入：
    无。
输出：
    无。
*/
void http_conn::unmap()
{
    if (m_file_address)
    {
        munmap(m_file_address, m_file_stat.st_size);
        m_file_address = 0;
    }
}
/*
作用：
    把已经准备好的 HTTP 响应写回客户端。
输入：
    无，直接读取成员变量中的写缓冲区、iovec 和剩余字节数。
输出：
    true：本次发送成功，或者因为 EAGAIN 需要等待下次可写事件继续发。
    false：发送失败，连接应被关闭。
*/
bool http_conn::write()
{
    int temp = 0;

    if (bytes_to_send == 0)
    {
        // 响应已经发完，重新监听读事件，准备接下一次请求。
        modfd(m_epollfd, m_sockfd, EPOLLIN, m_TRIGMode);
        init();
        return true;
    }

    while (1)
    {
        // writev 把“响应头 + 文件内容”一起发出去，减少系统调用开销。
        // writev 可以一次发送多段缓冲区：常见场景是“响应头 + 文件内容”一起发。
        temp = writev(m_sockfd, m_iv, m_iv_count);

        if (temp < 0)
        {
            if (errno == EAGAIN)
            {
                // 内核发送缓冲区暂时写满，等下次可写事件再继续发。
                modfd(m_epollfd, m_sockfd, EPOLLOUT, m_TRIGMode);
                return true;
            }
            unmap();
            return false;
        }

        // temp 是本次真正发出去的总字节数，更新整体发送进度。
        bytes_have_send += temp;
        bytes_to_send -= temp;
        // 如果响应头已经全部发送完，就开始计算文件内容部分还剩多少。
        if (bytes_have_send >= m_iv[0].iov_len)
        {
            // 响应头已经全部发完，后面只剩文件内容要继续发送。
            m_iv[0].iov_len = 0;
            m_iv[1].iov_base = m_file_address + (bytes_have_send - m_write_idx);
            m_iv[1].iov_len = bytes_to_send;
        }
        else
        {
            // 响应头都还没发完，把头缓冲区前移到尚未发送的位置。
            m_iv[0].iov_base = m_write_buf + bytes_have_send;
            m_iv[0].iov_len = m_iv[0].iov_len - bytes_have_send;
        }

        if (bytes_to_send <= 0)
        {
            unmap();
            modfd(m_epollfd, m_sockfd, EPOLLIN, m_TRIGMode);

            if (m_linger)
            {
                // keep-alive 连接不断开，只重置解析状态继续复用这个连接对象。
                init();
                return true;
            }
            else
            {
                // 非 keep-alive 连接，通知上层后续关闭 socket。
                return false;
            }
        }
    }
}
/*
作用：
    向响应写缓冲区追加格式化文本。
输入：
    format 和可变参数：要拼进响应报文的内容。
输出：
    true：追加成功。
    false：写缓冲区空间不足。
*/
bool http_conn::add_response(const char *format, ...)
{
    if (m_write_idx >= WRITE_BUFFER_SIZE)
        return false;
    va_list arg_list;
    va_start(arg_list, format);
    // vsnprintf 按格式化字符串把内容安全写入缓冲区，并避免越界。
    int len = vsnprintf(m_write_buf + m_write_idx, WRITE_BUFFER_SIZE - 1 - m_write_idx, format, arg_list);
    if (len >= (WRITE_BUFFER_SIZE - 1 - m_write_idx))
    {
        va_end(arg_list);
        return false;
    }
    m_write_idx += len;
    va_end(arg_list);

    LOG_INFO("request:%s", m_write_buf);

    return true;
}
/*
作用：
    追加 HTTP 状态行，例如 “HTTP/1.1 200 OK”。
输入：
    status：状态码。
    title：状态码对应的文本说明。
输出：
    true / false：是否成功写入缓冲区。
*/
bool http_conn::add_status_line(int status, const char *title)
{
    return add_response("%s %d %s\r\n", "HTTP/1.1", status, title);
}
/*
作用：
    统一追加常见响应头。
输入：
    content_len：响应体长度。
输出：
    true / false：是否写入成功。
*/
bool http_conn::add_headers(int content_len)
{
    return add_content_length(content_len) && add_linger() &&
           add_blank_line();
}
bool http_conn::add_content_length(int content_len)
{
    return add_response("Content-Length:%d\r\n", content_len);
}
bool http_conn::add_content_type()
{
    return add_response("Content-Type:%s\r\n", "text/html");
}
bool http_conn::add_linger()
{
    return add_response("Connection:%s\r\n", (m_linger == true) ? "keep-alive" : "close");
}
bool http_conn::add_blank_line()
{
    return add_response("%s", "\r\n");
}
bool http_conn::add_content(const char *content)
{
    return add_response("%s", content);
}
/*
作用：
    根据业务处理结果拼装完整的 HTTP 响应。
输入：
    ret：业务处理阶段返回的 HTTP_CODE。
输出：
    true：响应已经成功组织好。
    false：组织响应失败。
*/
bool http_conn::process_write(HTTP_CODE ret)
{
    // 这个函数不直接往 socket 发数据。
    // 它只负责根据 ret 的不同，把 HTTP 响应“准备好”。
    // 真正发送发生在 write() 里。
    switch (ret)
    {
    case INTERNAL_ERROR:
    {
        // 500：服务器内部错误。
        add_status_line(500, error_500_title);
        add_headers(strlen(error_500_form));
        if (!add_content(error_500_form))
            return false;
        break;
    }
    case BAD_REQUEST:
    {
        // 源码这里把 BAD_REQUEST 组织成了 404 风格页面。
        add_status_line(404, error_404_title);
        add_headers(strlen(error_404_form));
        if (!add_content(error_404_form))
            return false;
        break;
    }
    case FORBIDDEN_REQUEST:
    {
        // 403：有资源，但没有权限访问。
        add_status_line(403, error_403_title);
        add_headers(strlen(error_403_form));
        if (!add_content(error_403_form))
            return false;
        break;
    }
    case FILE_REQUEST:
    {
        // 200：正常文件请求。
        add_status_line(200, ok_200_title);
        if (m_file_stat.st_size != 0)
        {
            // 文件非空时，把响应分成两段：
            // 1. m_write_buf 存响应头
            // 2. m_file_address 指向 mmap 后的文件内容
            add_headers(m_file_stat.st_size);
            // iovec 描述一段待发送内存：iov_base 是地址，iov_len 是长度。
            m_iv[0].iov_base = m_write_buf;
            m_iv[0].iov_len = m_write_idx;
            m_iv[1].iov_base = m_file_address;
            m_iv[1].iov_len = m_file_stat.st_size;
            m_iv_count = 2;
            // 后续 write() 会根据这个总字节数循环发送。
            bytes_to_send = m_write_idx + m_file_stat.st_size;
            return true;
        }
        else
        {
            // 空文件时返回一个最简单的空 html。
            const char *ok_string = "<html><body></body></html>";
            add_headers(strlen(ok_string));
            if (!add_content(ok_string))
                return false;
        }
    }
    default:
        return false;
    }
    // 走到这里说明响应只需要发送文本缓冲区，不需要第二段文件内容。
    m_iv[0].iov_base = m_write_buf;
    m_iv[0].iov_len = m_write_idx;
    m_iv_count = 1;
    bytes_to_send = m_write_idx;
    return true;
}
/*
作用：
    单连接处理总入口。
输入：
    无。
输出：
    无。
关键逻辑：
    1. 先解析请求；
    2. 如果请求还不完整，就重新监听可读事件；
    3. 如果请求完整，就生成响应并切换到可写事件。
*/
void http_conn::process()
{
    // 单连接处理主链：解析请求、组织响应，然后切换到可写事件。
    HTTP_CODE read_ret = process_read();
    if (read_ret == NO_REQUEST)
    {
        // 请求还没收完整，例如只收到半个请求头或 body 还没到齐。
        modfd(m_epollfd, m_sockfd, EPOLLIN, m_TRIGMode);
        return;
    }
    // 这里只是先把响应组织好，真正写 socket 的动作要等 EPOLLOUT 时再执行。
    bool write_ret = process_write(read_ret);
    if (!write_ret)
    {
        close_conn();
    }
    // 响应已经组织完成，切到可写事件，等真正发送。
    modfd(m_epollfd, m_sockfd, EPOLLOUT, m_TRIGMode);
}
