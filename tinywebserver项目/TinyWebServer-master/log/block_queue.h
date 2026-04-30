/*************************************************************
* 循环数组实现的阻塞队列：
* m_back = (m_back + 1) % m_max_size;
* 线程安全：每个操作前先加锁，操作完成后再解锁。
**************************************************************/

#ifndef BLOCK_QUEUE_H
#define BLOCK_QUEUE_H

#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include "../lock/locker.h"
using namespace std;

template <class T>
class block_queue
{
public:
    // 作用：构造循环阻塞队列。
    block_queue(int max_size = 1000)
    {
        if (max_size <= 0)
        {
            exit(-1);
        }

        m_max_size = max_size;
        m_array = new T[max_size];
        m_size = 0;
        m_front = -1;
        m_back = -1;
    }

    // 作用：清空队列状态，不释放底层数组。
    void clear()
    {
        m_mutex.lock();
        m_size = 0;
        m_front = -1;
        m_back = -1;
        m_mutex.unlock();
    }

    // 作用：析构队列并释放底层数组。
    ~block_queue()
    {
        m_mutex.lock();
        if (m_array != NULL)
            delete[] m_array;

        m_mutex.unlock();
    }

    // 判断队列是否已满。
    bool full()
    {
        m_mutex.lock();
        if (m_size >= m_max_size)
        {
            m_mutex.unlock();
            return true;
        }
        m_mutex.unlock();
        return false;
    }

    // 判断队列是否为空。
    bool empty()
    {
        m_mutex.lock();
        if (0 == m_size)
        {
            m_mutex.unlock();
            return true;
        }
        m_mutex.unlock();
        return false;
    }

    // 读取队首元素，但不弹出。
    bool front(T &value)
    {
        m_mutex.lock();
        if (0 == m_size)
        {
            m_mutex.unlock();
            return false;
        }
        value = m_array[m_front];
        m_mutex.unlock();
        return true;
    }

    // 读取队尾元素，但不弹出。
    bool back(T &value)
    {
        m_mutex.lock();
        if (0 == m_size)
        {
            m_mutex.unlock();
            return false;
        }
        value = m_array[m_back];
        m_mutex.unlock();
        return true;
    }

    // 作用：返回当前元素个数。
    int size()
    {
        int tmp = 0;

        m_mutex.lock();
        tmp = m_size;

        m_mutex.unlock();
        return tmp;
    }

    // 作用：返回队列容量上限。
    int max_size()
    {
        int tmp = 0;

        m_mutex.lock();
        tmp = m_max_size;

        m_mutex.unlock();
        return tmp;
    }

    // 往队列添加元素。
    // 对日志场景来说，相当于生产者投递一条新日志。
    bool push(const T &item)
    {
        m_mutex.lock();
        if (m_size >= m_max_size)
        {
            m_cond.broadcast();
            m_mutex.unlock();
            return false;
        }

        // 循环队列尾指针前移。
        m_back = (m_back + 1) % m_max_size;
        m_array[m_back] = item;
        m_size++;

        // 通知可能正在等待的消费者线程。
        m_cond.broadcast();
        m_mutex.unlock();
        return true;
    }

    // 阻塞弹出一个元素。
    // 如果当前队列为空，就等待条件变量。
    bool pop(T &item)
    {
        m_mutex.lock();
        while (m_size <= 0)
        {
            if (!m_cond.wait(m_mutex.get()))
            {
                m_mutex.unlock();
                return false;
            }
        }

        // 循环队列头指针前移。
        m_front = (m_front + 1) % m_max_size;
        item = m_array[m_front];
        m_size--;
        m_mutex.unlock();
        return true;
    }

    // 带超时的弹出。
    bool pop(T &item, int ms_timeout)
    {
        struct timespec t = {0, 0};
        struct timeval now = {0, 0};
        gettimeofday(&now, NULL);
        m_mutex.lock();
        if (m_size <= 0)
        {
            t.tv_sec = now.tv_sec + ms_timeout / 1000;
            t.tv_nsec = (ms_timeout % 1000) * 1000;
            if (!m_cond.timewait(m_mutex.get(), t))
            {
                m_mutex.unlock();
                return false;
            }
        }

        if (m_size <= 0)
        {
            m_mutex.unlock();
            return false;
        }

        m_front = (m_front + 1) % m_max_size;
        item = m_array[m_front];
        m_size--;
        m_mutex.unlock();
        return true;
    }

private:
    // 保护队列内部数组和下标的互斥锁。
    locker m_mutex;
    // 队列为空时，消费者在这个条件变量上等待。
    cond m_cond;

    // 循环数组底层存储空间。
    T *m_array;
    // 当前队列里实际有多少元素。
    int m_size;
    // 队列最大容量。
    int m_max_size;
    // 队首下标。
    // 注意这里记录的是“上一次弹出的位置”，真正队首在 (m_front + 1) % m_max_size。
    int m_front;
    // 队尾下标，指向最后一个已插入元素的位置。
    int m_back;
};

#endif
