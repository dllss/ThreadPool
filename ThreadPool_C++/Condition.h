#ifndef _CONDITION_H_
#define _CONDITION_H_

#include <pthread.h>

//封装一个互斥量和条件变量作为状态
typedef struct Condition
{
    pthread_mutex_t pmutex;
    pthread_cond_t pcond;
}Condition_t;

//对状态的操作函数
int Condition_init(Condition_t *cond);
int Condition_lock(Condition_t *cond);
int Condition_unlock(Condition_t *cond);
int Condition_wait(Condition_t *cond);
int Condition_timedwait(Condition_t *cond, const struct timespec *abstime);
int Condition_signal(Condition_t* cond);
int Condition_broadcast(Condition_t *cond);
int Condition_destroy(Condition_t *cond);

#endif