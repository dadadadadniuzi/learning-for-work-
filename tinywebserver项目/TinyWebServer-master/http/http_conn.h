#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H
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
#include <map>

#include "../lock/locker.h"
#include "../CGImysql/sql_connection_pool.h"
#include "../timer/lst_timer.h"
#include "../log/log.h"

class http_conn
{
public:
    // 目标文件绝对路径缓冲区的最大长度。
    static const int FILENAME_LEN = 200;
    // 读缓冲区大小：socket 收到的 HTTP 原始请求数据先写到这里。
    static const int READ_BUFFER_SIZE = 2048;
    // 写缓冲区大小：HTTP 响应头和少量文本内容先拼在这里。
    static const int WRITE_BUFFER_SIZE = 1024;

    enum METHOD
    {
        // GET：最常见的请求方法，一般用于获取页面或静态资源。
        GET = 0,
        // POST：带请求体的方法，这个项目主要用于登录和注册。
        POST,
        // HEAD：只要响应头，不要响应体。
        HEAD,
        // PUT：常用于更新资源。
        PUT,
        // DELETE：常用于删除资源。
        DELETE,
        // TRACE：用于回显请求链路，更多见于调试场景。
        TRACE,
        // OPTIONS：询问服务器支持哪些 HTTP 方法。
        OPTIONS,
        // CONNECT：代理场景中较常见。
        CONNECT,
        // 源码里写成 PATH，语义上通常更像 PATCH。
        PATH
    };

    enum CHECK_STATE
    {
        // 主状态机当前正在解析请求行，例如：GET /index.html HTTP/1.1
        CHECK_STATE_REQUESTLINE = 0,
        // 主状态机当前正在解析请求头。
        CHECK_STATE_HEADER,
        // 主状态机当前正在解析请求体。
        CHECK_STATE_CONTENT
    };

    enum HTTP_CODE
    {
        // 请求还没收完整，不能进入业务处理。
        NO_REQUEST,
        // 请求已经完整，可以进入 do_request 处理。
        GET_REQUEST,
        // 请求格式错误，例如方法、版本、URL 不合法。
        BAD_REQUEST,
        // 请求资源不存在。
        NO_RESOURCE,
        // 请求资源存在，但权限不足。
        FORBIDDEN_REQUEST,
        // 目标文件或业务结果已经准备好，可以正常响应。
        FILE_REQUEST,
        // 服务器内部错误。
        INTERNAL_ERROR,
        // 连接已经关闭。
        CLOSED_CONNECTION
    };

    enum LINE_STATUS
    {
        // 成功解析出一整行，以 \r\n 结束。
        LINE_OK = 0,
        // 这一行格式非法。
        LINE_BAD,
        // 还没有收完整一行，需要继续读取 socket。
        LINE_OPEN
    };

public:
    http_conn() {}
    ~http_conn() {}

public:
    /*
    作用：
        初始化单个 HTTP 连接对象，并把该连接注册进 epoll。
    输入：
        sockfd：客户端连接 fd。
        addr：客户端地址。
        char *：网站根目录。
        int：触发模式。
        int：是否关闭日志。
        user/passwd/sqlname：数据库认证信息。
    输出：
        无。
    */
    void init(int sockfd, const sockaddr_in &addr, char *, int, int, string user, string passwd, string sqlname);

    /*
    作用：
        关闭连接，并在需要时把该 fd 从 epoll 中移除。
    输入：
        real_close：是否真正关闭底层 socket。
    输出：
        无。
    */
    void close_conn(bool real_close = true);

    // 作用：单连接请求处理总入口，内部完成“解析请求 -> 组织响应 -> 注册可写事件”。
    void process();

    // 作用：从 socket 把数据读入读缓冲区。
    // 输出：true 表示成功读到数据，false 表示连接关闭或读取失败。
    bool read_once();

    // 作用：把已经准备好的响应写回客户端。
    // 输出：true 表示本次写成功或可继续保持连接，false 表示写失败或应关闭连接。
    bool write();

    // 返回当前客户端地址结构体地址，方便外部打印 IP 等信息。
    sockaddr_in *get_address()
    {
        return &m_address;
    }

    // 作用：启动时读取数据库 user 表，把用户名密码加载到内存 map。
    void initmysql_result(connection_pool *connPool);

    // timer_flag：
    // Reactor 模式下由工作线程回写。
    // 1 表示本轮处理后应关闭连接。
    // 0 表示连接仍可继续使用。
    int timer_flag;

    // improv：
    // Reactor 模式下表示“工作线程是否已经处理完成”。
    // 主线程会轮询这个值来等待子线程结果。
    int improv;

private:
    // 作用：重置连接对象状态，供新连接初始化或 keep-alive 场景复用。
    void init();

    // 作用：HTTP 解析总调度函数。
    // 输出：返回当前请求解析到的阶段结果，例如 NO_REQUEST / FILE_REQUEST / BAD_REQUEST。
    HTTP_CODE process_read();

    // 作用：根据业务处理结果拼接 HTTP 响应报文。
    bool process_write(HTTP_CODE ret);

    // 作用：解析请求行，提取方法、URL 和 HTTP 版本。
    HTTP_CODE parse_request_line(char *text);

    // 作用：解析请求头。
    HTTP_CODE parse_headers(char *text);

    // 作用：解析请求体，主要服务于 POST 表单。
    HTTP_CODE parse_content(char *text);

    // 作用：真正的业务入口，决定访问静态文件还是执行登录注册逻辑。
    HTTP_CODE do_request();

    // 返回当前正在解析的这一行在读缓冲区中的起始地址。
    char *get_line() { return m_read_buf + m_start_line; };

    // 作用：从 TCP 字节流中切出一整行 HTTP 文本。
    LINE_STATUS parse_line();

    // 作用：解除文件的 mmap 映射，避免内存映射长期占用。
    void unmap();

    // 作用：向写缓冲区追加格式化内容。
    bool add_response(const char *format, ...);

    // 作用：向响应体中追加普通文本内容。
    bool add_content(const char *content);

    // 作用：追加 HTTP 状态行。
    bool add_status_line(int status, const char *title);

    // 作用：追加响应头部常见字段。
    bool add_headers(int content_length);

    // 作用：追加 Content-Type。
    bool add_content_type();

    // 作用：追加 Content-Length。
    bool add_content_length(int content_length);

    // 作用：追加 Connection 字段，决定 keep-alive 或 close。
    bool add_linger();

    // 作用：追加头部结束所需的空行。
    bool add_blank_line();

public:
    // 所有 http_conn 对象共享同一个 epoll 实例 fd。
    static int m_epollfd;

    // 所有连接共享的在线连接总数。
    static int m_user_count;

    // 当前连接正在使用的 MySQL 连接句柄。
    MYSQL *mysql;

    // 当前任务类型标记：
    // 0 表示本轮处理的是读任务
    // 1 表示本轮处理的是写任务
    int m_state;

private:
    // 当前连接对应的 socket 文件描述符。
    int m_sockfd;

    // 客户端地址，包含 IP 和端口。
    sockaddr_in m_address;

    // 读缓冲区：原始 HTTP 请求报文先进入这里。
    char m_read_buf[READ_BUFFER_SIZE];

    // 当前读缓冲区中“已经读到了多少字节”。
    // 新一次 recv 会从 m_read_buf + m_read_idx 位置继续往后写。
    long m_read_idx;

    // 当前已经检查到读缓冲区的哪个位置。
    // parse_line / process_read 会不断推进这个索引。
    long m_checked_idx;

    // 当前正在解析的这一行，在读缓冲区中的起始下标。
    int m_start_line;

    // 写缓冲区：HTTP 响应头和少量正文会先拼装到这里。
    char m_write_buf[WRITE_BUFFER_SIZE];

    // 当前写缓冲区里已经写入了多少字节。
    int m_write_idx;

    // 主状态机当前所处阶段。
    CHECK_STATE m_check_state;

    // 当前请求的方法，例如 GET / POST。
    METHOD m_method;

    // 最终要访问的目标文件绝对路径。
    char m_real_file[FILENAME_LEN];

    // 指向请求 URL 在读缓冲区中的位置，例如 "/index.html"。
    char *m_url;

    // 指向 HTTP 版本字符串，例如 "HTTP/1.1"。
    char *m_version;

    // 指向 Host 头部的值。
    char *m_host;

    // 请求体长度，对应 Content-Length。
    long m_content_length;

    // 是否保持长连接。
    // true 表示 keep-alive
    // false 表示响应完成后关闭连接
    bool m_linger;

    // mmap 映射后的文件内存地址。
    char *m_file_address;

    // 目标文件的状态信息，包含大小、权限、类型等。
    struct stat m_file_stat;

    // writev 使用的 iovec 数组。
    // 通常 m_iv[0] 是响应头，m_iv[1] 是文件内容。
    struct iovec m_iv[2];

    // 当前有多少段 iovec 需要发送。
    int m_iv_count;

    // 是否启用 CGI 风格业务处理标记。
    // 在这个项目里主要表示是否进入 POST 登录/注册逻辑。
    int cgi;

    // 指向请求体内容起始位置。
    // POST 表单解析用户名密码时会从这里读。
    char *m_string;

    // 当前还剩多少字节没有发送给客户端。
    int bytes_to_send;

    // 当前已经发送了多少字节。
    int bytes_have_send;

    // 网站根目录，例如 .../TinyWebServer-master/root
    char *doc_root;

    // 启动时从数据库预加载到内存的 用户名 -> 密码 映射表。
    map<string, string> m_users;

    // 当前连接采用 LT 还是 ET 触发模式。
    int m_TRIGMode;

    // 是否关闭日志功能。
    int m_close_log;

    // 数据库用户名配置副本。
    char sql_user[100];

    // 数据库密码配置副本。
    char sql_passwd[100];

    // 数据库名称配置副本。
    char sql_name[100];
};

#endif
