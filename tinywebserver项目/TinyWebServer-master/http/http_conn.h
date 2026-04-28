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
    static const int FILENAME_LEN = 200;
    static const int READ_BUFFER_SIZE = 2048;
    static const int WRITE_BUFFER_SIZE = 1024;
    enum METHOD
    {
        GET = 0,
        POST,
        HEAD,
        PUT,
        DELETE,
        TRACE,
        OPTIONS,
        CONNECT,
        PATH
    };
    enum CHECK_STATE
    {
        CHECK_STATE_REQUESTLINE = 0,
        CHECK_STATE_HEADER,
        CHECK_STATE_CONTENT
    };
    enum HTTP_CODE
    {
        NO_REQUEST,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURCE,
        FORBIDDEN_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION
    };
    enum LINE_STATUS
    {
        LINE_OK = 0,
        LINE_BAD,
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
    sockaddr_in *get_address()
    {
        return &m_address;
    }
    // 作用：启动时读取数据库 user 表，把用户名密码加载到内存 map。
    void initmysql_result(connection_pool *connPool);
    int timer_flag;
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
    static int m_epollfd;
    static int m_user_count;
    MYSQL *mysql;
    int m_state;  //读为0, 写为1

private:
    int m_sockfd;
    sockaddr_in m_address;
    char m_read_buf[READ_BUFFER_SIZE];
    long m_read_idx;
    long m_checked_idx;
    int m_start_line;
    char m_write_buf[WRITE_BUFFER_SIZE];
    int m_write_idx;
    CHECK_STATE m_check_state;
    METHOD m_method;
    char m_real_file[FILENAME_LEN];
    char *m_url;
    char *m_version;
    char *m_host;
    long m_content_length;
    bool m_linger;
    char *m_file_address;
    struct stat m_file_stat;
    struct iovec m_iv[2];
    int m_iv_count;
    int cgi;        //是否启用的POST
    char *m_string; //存储请求头数据
    int bytes_to_send;
    int bytes_have_send;
    char *doc_root;

    map<string, string> m_users;
    int m_TRIGMode;
    int m_close_log;

    char sql_user[100];
    char sql_passwd[100];
    char sql_name[100];
};

#endif
