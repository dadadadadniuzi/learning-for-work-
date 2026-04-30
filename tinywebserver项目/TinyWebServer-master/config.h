#ifndef CONFIG_H
#define CONFIG_H

#include "webserver.h"

using namespace std;

class Config
{
public:
    /*
    作用：
        构造配置对象，并写入服务器启动时使用的默认参数。
    输入：
        无。
    输出：
        无。
    */
    Config();
    ~Config() {};

    /*
    作用：
        解析命令行参数，把用户输入的启动参数写回当前配置对象。
    输入：
        argc：命令行参数个数。
        argv：命令行参数数组。
    输出：
        无。解析结果直接更新 Config 的成员变量。
    */
    void parse_arg(int argc, char *argv[]);

    // 服务器监听端口号。
    int PORT;

    // 日志写入方式：
    // 0 同步日志
    // 1 异步日志
    int LOGWrite;

    // 总的 LT/ET 触发模式组合配置。
    int TRIGMode;

    // 监听 fd 的触发模式。
    int LISTENTrigmode;

    // 已连接 fd 的触发模式。
    int CONNTrigmode;

    // 是否启用优雅关闭连接。
    int OPT_LINGER;

    // 数据库连接池中的连接数量。
    int sql_num;

    // 线程池中的工作线程数量。
    int thread_num;

    // 是否关闭日志系统：
    // 0 表示开启日志
    // 1 表示关闭日志
    int close_log;

    // 并发模型选择：
    // 0 更接近 Proactor
    // 1 Reactor
    int actor_model;
};

#endif
