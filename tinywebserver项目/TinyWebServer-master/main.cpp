#include "config.h"

int main(int argc, char *argv[])
{
    // 建议先从 main 开始读，它把整个服务器的启动顺序完整串了起来。
    //需要修改的数据库信息,登录名,密码,库名
    string user = "root";
    string passwd = "root";
    string databasename = "qgydb";

    //命令行解析
    Config config;
    config.parse_arg(argc, argv);

    WebServer server;
    // 这一步只是把配置灌进 WebServer，真正的日志、线程池、监听等都在下面启动。

    //初始化
    server.init(config.PORT, user, passwd, databasename, config.LOGWrite, 
                config.OPT_LINGER, config.TRIGMode,  config.sql_num,  config.thread_num, 
                config.close_log, config.actor_model);
    

    //日志
    server.log_write();

    //数据库
    server.sql_pool();

    //线程池
    server.thread_pool();

    //触发模式
    server.trig_mode();

    //监听
    server.eventListen();
    // 调用这里后，程序就常驻在主事件循环中，持续处理连接和请求。

    //运行
    server.eventLoop();

    return 0;
}
