#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>
#include "../lock/locker.h"
#include "../CGImysql/sql_connection_pool.h"

template <typename T>
class threadpool
{
public:
    /*
    作用：
        创建线程池并启动固定数量的工作线程。
    输入：
        actor_model：并发模型，决定线程如何处理读写任务。
        connPool：数据库连接池指针。
        thread_number：线程数量。
        max_request：任务队列最大长度。
    输出：
        无。
    */
    threadpool(int actor_model, connection_pool *connPool, int thread_number = 8, int max_request = 10000);

    // 作用：销毁线程池对象，释放线程数组。
    ~threadpool();

    // 作用：向任务队列添加一个 Reactor 风格任务。
    // 输出：true 表示入队成功，false 表示队列已满。
    bool append(T *request, int state);

    // 作用：向任务队列添加一个 Proactor 风格任务。
    // 输出：true 表示入队成功，false 表示队列已满。
    bool append_p(T *request);

private:
    // 作用：pthread 线程入口，内部转调到 run()。
    static void *worker(void *arg);
    // 作用：工作线程主循环，不断等待任务并执行。
    void run();

private:
    // 线程池中的工作线程数量。
    int m_thread_number;
    // 任务队列允许缓存的最大请求数。
    int m_max_requests;
    // 线程数组，每个元素都是一个 pthread 线程 id。
    pthread_t *m_threads;
    // 任务队列，队列中每个元素都是一个待处理连接对象指针。
    std::list<T *> m_workqueue;
    // 保护任务队列的互斥锁，避免多个线程同时改队列。
    locker m_queuelocker;
    // 队列状态信号量：
    // 大于 0 表示队列里至少有任务可取，
    // 为 0 时工作线程会阻塞等待。
    sem m_queuestat;
    // 数据库连接池指针，工作线程处理业务时会借连接。
    connection_pool *m_connPool;
    // 并发模型切换标记：
    // 1 表示 Reactor 风格
    // 其他值更接近 Proactor 风格
    int m_actor_model;
};

template <typename T>
threadpool<T>::threadpool(int actor_model, connection_pool *connPool, int thread_number, int max_requests)
    : m_actor_model(actor_model), m_thread_number(thread_number), m_max_requests(max_requests), m_threads(NULL), m_connPool(connPool)
{
    if (thread_number <= 0 || max_requests <= 0)
        throw std::exception();
    m_threads = new pthread_t[m_thread_number];
    if (!m_threads)
        throw std::exception();
    for (int i = 0; i < thread_number; ++i)
    {
        if (pthread_create(m_threads + i, NULL, worker, this) != 0)
        {
            delete[] m_threads;
            throw std::exception();
        }
        if (pthread_detach(m_threads[i]))
        {
            delete[] m_threads;
            throw std::exception();
        }
    }
}

template <typename T>
threadpool<T>::~threadpool()
{
    delete[] m_threads;
}

template <typename T>
bool threadpool<T>::append(T *request, int state)
{
    m_queuelocker.lock();
    // 队列满时直接拒绝新任务，避免任务无限堆积。
    if (m_workqueue.size() >= m_max_requests)
    {
        m_queuelocker.unlock();
        return false;
    }
    request->m_state = state;
    // 把待处理连接对象压入任务队列尾部。
    m_workqueue.push_back(request);
    m_queuelocker.unlock();
    // 唤醒一个阻塞中的工作线程。
    m_queuestat.post();
    return true;
}

template <typename T>
bool threadpool<T>::append_p(T *request)
{
    m_queuelocker.lock();
    if (m_workqueue.size() >= m_max_requests)
    {
        m_queuelocker.unlock();
        return false;
    }
    m_workqueue.push_back(request);
    m_queuelocker.unlock();
    m_queuestat.post();
    return true;
}

template <typename T>
void *threadpool<T>::worker(void *arg)
{
    // pthread 只能从普通函数起线程，所以这里做静态转发。
    threadpool *pool = (threadpool *)arg;
    pool->run();
    return pool;
}

template <typename T>
void threadpool<T>::run()
{
    while (true)
    {
        // 工作线程平时阻塞在这里，直到任务队列里来了新任务才会被唤醒。
        // 工作线程平时阻塞在这里，直到有新任务进队列才被唤醒。
        m_queuestat.wait();
        m_queuelocker.lock();
        if (m_workqueue.empty())
        {
            m_queuelocker.unlock();
            continue;
        }
        // 取出队头任务，准备由当前工作线程独占处理。
        T *request = m_workqueue.front();
        m_workqueue.pop_front();
        m_queuelocker.unlock();
        if (!request)
            continue;
        if (1 == m_actor_model)
        {
            // Reactor 模式下，工作线程负责真正的 read/write，然后顺序处理这个请求。
            if (0 == request->m_state)
            {
                // m_state == 0 表示这是一个读任务。
                if (request->read_once())
                {
                    // improv=1 用来通知主线程：本轮任务已经被工作线程处理过。
                    request->improv = 1;
                    // 借一个数据库连接，作用域结束时自动归还。
                    connectionRAII mysqlcon(&request->mysql, m_connPool);
                    // 继续做 HTTP 解析、业务处理、响应构造。
                    request->process();
                }
                else
                {
                    // 读失败时，交给主线程后续关闭连接和清理定时器。
                    request->improv = 1;
                    request->timer_flag = 1;
                }
            }
            else
            {
                // m_state != 0 表示这是一个写任务。
                if (request->write())
                {
                    // 写成功后，同样通知主线程这一轮已完成。
                    request->improv = 1;
                }
                else
                {
                    // 写失败时同样标记给主线程做善后处理。
                    request->improv = 1;
                    request->timer_flag = 1;
                }
            }
        }
        else
        {
            // Proactor 风格下，主线程已经完成读，工作线程重点负责后续业务处理。
            // Proactor 路径下，主线程已经读完 socket，这里重点做业务处理。
            connectionRAII mysqlcon(&request->mysql, m_connPool);
            request->process();
        }
    }
}

#endif
