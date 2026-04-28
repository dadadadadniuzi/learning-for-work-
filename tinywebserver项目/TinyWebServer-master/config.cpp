#include "config.h"

/*
作用：
    构造配置对象，并写入服务器启动时使用的默认参数。
输入：
    无。
输出：
    无。
说明：
    如果启动程序时没有额外传命令行参数，就会直接使用这里的默认值。
*/
Config::Config()
{
    // 端口号，默认 9006。
    PORT = 9006;

    // 日志写入方式，默认同步日志。
    // 0：同步写日志
    // 1：异步写日志
    LOGWrite = 0;

    // 触发模式组合，默认 listenfd LT + connfd LT。
    // 0：listen LT + conn LT
    // 1：listen LT + conn ET
    // 2：listen ET + conn LT
    // 3：listen ET + conn ET
    TRIGMode = 0;

    // listenfd 触发模式，默认 LT。
    LISTENTrigmode = 0;

    // connfd 触发模式，默认 LT。
    CONNTrigmode = 0;

    // 是否启用优雅关闭，默认不启用。
    // 0：关闭 linger
    // 1：开启 linger
    OPT_LINGER = 0;

    // 数据库连接池中的连接数量，默认 8。
    sql_num = 8;

    // 线程池中的工作线程数量，默认 8。
    thread_num = 8;

    // 是否关闭日志，默认不关闭。
    // 0：开启日志
    // 1：关闭日志
    close_log = 0;

    // 并发模型，默认是 Proactor 风格。
    // 0：Proactor
    // 1：Reactor
    actor_model = 0;
}

/*
作用：
    解析命令行参数，把用户在启动服务器时传入的配置覆盖到当前 Config 对象里。
输入：
    argc：命令行参数总数。
    argv：命令行参数数组。
输出：
    无。
说明：
    例如可以这样启动：
    ./server -p 9006 -l 1 -m 3 -o 1 -s 8 -t 8 -c 0 -a 0
    这段函数本质上做的是：
    “从命令行读字符串参数 -> 转成整数 -> 写回配置对象成员变量”。
*/
void Config::parse_arg(int argc, char *argv[])
{
    // opt 用来接收 getopt 每一轮解析出的“选项字符”。
    // 例如命令行里读到 -p 时，opt 的值就是字符 'p'。
    int opt;

    // 这是 getopt 使用的“选项说明串”。
    // 格式规则：
    // 1. 每个字母表示一个可以识别的选项。
    // 2. 字母后面跟冒号 : ，表示这个选项后面必须再带一个值。
    //
    // 所以这串 "p:l:m:o:s:t:c:a:" 的含义就是：
    // -p 要带值，例如 -p 9006
    // -l 要带值，例如 -l 1
    // -m 要带值，例如 -m 3
    // 其余同理。
    const char *str = "p:l:m:o:s:t:c:a:";

    // getopt 是 Linux/Unix 下常见的命令行解析函数。
    // 它会反复扫描 argv：
    // 1. 每次取出一个选项，例如 -p
    // 2. 返回该选项对应的字符，例如 'p'
    // 3. 如果该选项要求参数，那么它会把参数地址放进全局变量 optarg
    // 4. 当所有参数都处理完以后，getopt 返回 -1
    while ((opt = getopt(argc, argv, str)) != -1)
    {
        switch (opt)
        {
        case 'p':
        {
            // optarg 指向当前选项后面的参数字符串。
            // 例如命令行是：-p 9006
            // 那么此时 optarg 指向字符串 "9006"。
            //
            // atoi 的全称可以理解为 ascii to integer，
            // 作用是把字符串数字转成 int：
            // "9006" -> 9006
            // "8"    -> 8
            PORT = atoi(optarg);
            break;
        }
        case 'l':
        {
            // LOGWrite：
            // 0 表示同步日志
            // 1 表示异步日志
            LOGWrite = atoi(optarg);
            break;
        }
        case 'm':
        {
            // TRIGMode：
            // 0：listen LT + conn LT
            // 1：listen LT + conn ET
            // 2：listen ET + conn LT
            // 3：listen ET + conn ET
            TRIGMode = atoi(optarg);
            break;
        }
        case 'o':
        {
            // OPT_LINGER：
            // 0：不启用优雅关闭
            // 1：启用优雅关闭
            OPT_LINGER = atoi(optarg);
            break;
        }
        case 's':
        {
            // sql_num：数据库连接池大小。
            sql_num = atoi(optarg);
            break;
        }
        case 't':
        {
            // thread_num：线程池中的工作线程数。
            thread_num = atoi(optarg);
            break;
        }
        case 'c':
        {
            // close_log：
            // 0：开启日志
            // 1：关闭日志
            close_log = atoi(optarg);
            break;
        }
        case 'a':
        {
            // actor_model：
            // 0：Proactor 风格
            // 1：Reactor 风格
            actor_model = atoi(optarg);
            break;
        }
        default:
        {
            // 默认分支表示：
            // 当前参数不是程序关心的合法选项，直接忽略。
            break;
        }
        }
    }
}
