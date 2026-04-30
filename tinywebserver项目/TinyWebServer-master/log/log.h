#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <iostream>
#include <string>
#include <stdarg.h>
#include <pthread.h>
#include "block_queue.h"

using namespace std;

class Log
{
public:
    // 单例入口：整个进程共用一个日志对象。
    static Log *get_instance()
    {
        static Log instance;
        return &instance;
    }

    // 异步日志线程入口函数。
    static void *flush_log_thread(void *args)
    {
        Log::get_instance()->async_write_log();
        return NULL;
    }

    /*
    作用：
        初始化日志系统，并决定采用同步还是异步写日志。
    输入：
        file_name：日志文件名前缀。
        close_log：是否关闭日志。
        log_buf_size：单条日志缓冲区大小。
        split_lines：单个日志文件允许的最大行数。
        max_queue_size：异步队列大小，0 表示同步日志。
    输出：
        true 表示初始化成功，false 表示打开日志文件失败。
    */
    bool init(const char *file_name, int close_log, int log_buf_size = 8192, int split_lines = 5000000, int max_queue_size = 0);

    // 作用：写入一条日志，内部根据配置决定直接写文件还是先进阻塞队列。
    void write_log(int level, const char *format, ...);

    // 作用：强制刷新日志文件缓冲区。
    void flush(void);

private:
    Log();
    virtual ~Log();

    void *async_write_log()
    {
        string single_log;
        // 从阻塞队列中不断取出日志字符串并写入文件。
        while (m_log_queue->pop(single_log))
        {
            m_mutex.lock();
            fputs(single_log.c_str(), m_fp);
            m_mutex.unlock();
        }
        return NULL;
    }

private:
    // 日志文件所在目录名。
    char dir_name[128];
    // 日志文件名。
    char log_name[128];
    // 单个日志文件允许写入的最大行数，超过后会切分新文件。
    int m_split_lines;
    // 单条日志格式化缓冲区大小。
    int m_log_buf_size;
    // 当前已经写入的日志总行数。
    long long m_count;
    // 记录当前是哪一天，用于按天切分日志文件。
    int m_today;
    // 当前打开的日志文件指针。
    FILE *m_fp;
    // 单条日志临时格式化缓冲区。
    char *m_buf;
    // 异步日志使用的阻塞队列。
    block_queue<string> *m_log_queue;
    // 是否采用异步日志模式：
    // true 表示异步
    // false 表示同步
    bool m_is_async;
    // 保护日志文件写入的互斥锁。
    locker m_mutex;
    // 是否关闭日志。
    int m_close_log;
};

#define LOG_DEBUG(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(0, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_INFO(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(1, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_WARN(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(2, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_ERROR(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(3, format, ##__VA_ARGS__); Log::get_instance()->flush();}

#endif
