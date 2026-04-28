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
    ~Config(){};

    /*
    作用：
        解析命令行参数，把用户输入的启动参数写回当前配置对象。
    输入：
        argc：命令行参数个数。
        argv：命令行参数数组。
    输出：
        无。解析结果直接更新 Config 的成员变量。
    */
    void parse_arg(int argc, char*argv[]);

    //端口号
    int PORT;

    //日志写入方式
    int LOGWrite;

    //触发组合模式
    int TRIGMode;

    //listenfd触发模式
    int LISTENTrigmode;

    //connfd触发模式
    int CONNTrigmode;

    //优雅关闭链接
    int OPT_LINGER;

    //数据库连接池数量
    int sql_num;

    //线程池内的线程数量
    int thread_num;

    //是否关闭日志
    int close_log;

    //并发模型选择
    int actor_model;
};

#endif
