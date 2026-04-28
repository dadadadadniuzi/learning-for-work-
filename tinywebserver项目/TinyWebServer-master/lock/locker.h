#ifndef LOCKER_H
#define LOCKER_H

#include <exception>
#include <pthread.h>
#include <semaphore.h>

class sem
{
public:
    // 作用：初始化一个值为 0 的信号量。
    sem()
    {
        if (sem_init(&m_sem, 0, 0) != 0)
        {
            throw std::exception();
        }
    }
    // 作用：初始化一个值为 num 的信号量。
    sem(int num)
    {
        if (sem_init(&m_sem, 0, num) != 0)
        {
            throw std::exception();
        }
    }
    ~sem()
    {
        sem_destroy(&m_sem);
    }
    // 作用：P 操作，资源不足时阻塞等待。
    bool wait()
    {
        return sem_wait(&m_sem) == 0;
    }
    // 作用：V 操作，释放一个资源并唤醒等待线程。
    bool post()
    {
        return sem_post(&m_sem) == 0;
    }

private:
    sem_t m_sem;
};
class locker
{
public:
    // 作用：构造互斥锁。
    locker()
    {
        if (pthread_mutex_init(&m_mutex, NULL) != 0)
        {
            throw std::exception();
        }
    }
    ~locker()
    {
        pthread_mutex_destroy(&m_mutex);
    }
    // 作用：加锁，保护临界区。
    bool lock()
    {
        return pthread_mutex_lock(&m_mutex) == 0;
    }
    // 作用：解锁，离开临界区。
    bool unlock()
    {
        return pthread_mutex_unlock(&m_mutex) == 0;
    }
    // 作用：返回底层 pthread_mutex_t 指针，供条件变量等待函数使用。
    pthread_mutex_t *get()
    {
        return &m_mutex;
    }

private:
    pthread_mutex_t m_mutex;
};
class cond
{
public:
    // 作用：构造条件变量。
    cond()
    {
        if (pthread_cond_init(&m_cond, NULL) != 0)
        {
            //pthread_mutex_destroy(&m_mutex);
            throw std::exception();
        }
    }
    ~cond()
    {
        pthread_cond_destroy(&m_cond);
    }
    // 作用：等待条件成立，等待期间自动释放互斥锁。
    bool wait(pthread_mutex_t *m_mutex)
    {
        int ret = 0;
        //pthread_mutex_lock(&m_mutex);
        ret = pthread_cond_wait(&m_cond, m_mutex);
        //pthread_mutex_unlock(&m_mutex);
        return ret == 0;
    }
    // 作用：带超时时间的条件等待。
    bool timewait(pthread_mutex_t *m_mutex, struct timespec t)
    {
        int ret = 0;
        //pthread_mutex_lock(&m_mutex);
        ret = pthread_cond_timedwait(&m_cond, m_mutex, &t);
        //pthread_mutex_unlock(&m_mutex);
        return ret == 0;
    }
    // 作用：唤醒一个等待该条件变量的线程。
    bool signal()
    {
        return pthread_cond_signal(&m_cond) == 0;
    }
    // 作用：唤醒所有等待该条件变量的线程。
    bool broadcast()
    {
        return pthread_cond_broadcast(&m_cond) == 0;
    }

private:
    //static pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
};
#endif
