#include "Condition.h"

//初始化
int Condition_init(Condition_t *cond)
{
    int status;
    if((status = pthread_mutex_init(&cond->pmutex, NULL)))
        return status;
    
    if((status = pthread_cond_init(&cond->pcond, NULL)))
        return status;
    
    return 0;
}

//加锁
int Condition_lock(Condition_t *cond)
{
    return pthread_mutex_lock(&cond->pmutex);
}

//解锁
int Condition_unlock(Condition_t *cond)
{
    return pthread_mutex_unlock(&cond->pmutex);
}

//等待
int Condition_wait(Condition_t *cond)
{
    return pthread_cond_wait(&cond->pcond, &cond->pmutex);
}

//固定时间等待
int Condition_timedwait(Condition_t *cond, const struct timespec *abstime)
{
    return pthread_cond_timedwait(&cond->pcond, &cond->pmutex, abstime);
}

//唤醒一个睡眠线程
int Condition_signal(Condition_t* cond)
{
    return pthread_cond_signal(&cond->pcond);
}

//唤醒所有睡眠线程
int Condition_broadcast(Condition_t *cond)
{
    return pthread_cond_broadcast(&cond->pcond);
}

//释放
int Condition_destroy(Condition_t *cond)
{
    int status;
    if((status = pthread_mutex_destroy(&cond->pmutex)))
        return status;
    
    if((status = pthread_cond_destroy(&cond->pcond)))
        return status;
        
    return 0;
}