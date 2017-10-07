#include "ThreadPool.h"  
#include <string> 
//#include <queue> 
#include <iostream>  
#include <exception>  
#include <unistd.h>
#include <functional>

using namespace std;  
  

Task::Task(string taskName)  
{  
   this->m_TaskName = taskName;  
   m_ptrData = NULL;  
}    
void Task::SetData(void* data){  
  m_ptrData = data;  
}  
   

ThreadPool::ThreadPool(int maxThreadNum)  
{  
  this->maxThreadNum = maxThreadNum; 
  this->quit=0;  
  this->runpos=0;
  Condition_init(&condition); 
}
int ThreadPool::AddTask(Task *task)  
{  
  Condition_lock(&condition);
  queueTaskList.push(task);
  //线程池中有线程空闲，唤醒
  if( IdleThreadNum() > 0)
  {
    Condition_signal(&condition);
  }
  else if(IdleThreadNum()+BusyThreadNum() < this->maxThreadNum)//当前线程池中线程个数没有达到设定的最大值，创建一个新的线性
  {
    pthread_t tid;
    pthread_create(&tid, NULL, ThreadFunc, (void*)this);
    vecBusyThread.push_back(tid);
    cout<<"Create Thread id : "<<tid<<endl;
  }  
  Condition_unlock(&condition);
  return 0;  
}  

void* ThreadPool::ThreadFunc(void *arg)  
{  
  ThreadPool *ths=(ThreadPool*)arg;
  queue<Task*>* taskList=&ths->queueTaskList;

  struct timespec abstime;
  int timeout;
  pthread_t tid = pthread_self();  
  while(1)  
  {  
    timeout = 0;
    //访问线程池之前需要加锁
    Condition_lock(&ths->condition);
    ths->MoveToIdle(tid);
    //cout << "thread :" << tid << " is idle" << endl;
    //cout << "task left : " << ths->TaskNum() <<endl;
    //等待队列有任务到来 或者 收到线程池销毁通知
    while(ths->TaskNum()==0 && !ths->quit){
      //否则线程阻塞等待
      cout<<"Thread "<<tid<<" is waiting"<<endl;
      //获取从当前时间，并加上等待时间， 设置进程的超时睡眠时间
      clock_gettime(CLOCK_REALTIME,&abstime);
      abstime.tv_sec += 2;
      int status = Condition_timedwait(&ths->condition,&abstime);
      if(status == ETIMEDOUT)
      {
          cout<<"thread "<<tid<<" wait timed out"<<endl;
          timeout = 1;
          break;
      }
    }
    if(ths->TaskNum()>0){
      //取出等待队列最前的任务，移除任务，并执行任务
      Task *task=taskList->front();
      taskList->pop();
      //放入忙绿线程列表
      ths->MoveToBusy(tid);
      cout<<"Runnig pos : "<< (++ths->runpos) <<" ===>>  ";
      //由于任务执行需要消耗时间，先解锁让其他线程访问线程池
      Condition_unlock(&ths->condition);
      //执行任务
      cout << "thread :" << tid << " run" << endl;   
      task->Run();
      //执行完任务释放内存,这个内存是在main函数中申请的Task内存
      delete task;
      //重新加锁
      Condition_lock(&ths->condition);
    } 
    //退出线程池
    if(ths->quit && ths->TaskNum()==0)
    {
        ths->DelThread(tid);//当前工作的线程数-1
        //若线程池中没有线程，通知等待线程（主线程）全部任务已经完成
        if( ！ths->IdleThreadNum() && ！ths->BusyThreadNum() )
        {
            Condition_signal(&ths->condition);
            cout<<"All Task Finish !"<<endl;
        }
        Condition_unlock(&ths->condition);
        break;
    }
    //超时，跳出销毁线程
    if(timeout == 1)
    {
        ths->DelThread(tid);//当前工作的线程数-1
        Condition_unlock(&ths->condition);
        break;
    }
    Condition_unlock(&ths->condition);
  } 

  cout<<"thread "<<tid<<" is exiting"<<endl;
  return (void*)0;  
} 

void ThreadPool::DestroyAll()  
{  
  if(quit){
    return;
  }
  //加锁
  Condition_lock(&this->condition);
  //设置销毁标记为1
  this->quit = 1;
  //线程池中线程个数大于0
  if( IdleThreadNum() > 0 || BusyThreadNum() > 0)
  {
      //对于等待的线程，发送信号唤醒
      if(IdleThreadNum())
      {
          Condition_broadcast(&this->condition);
      }
      //正在执行任务的线程，等待他们结束任务
      while(BusyThreadNum() || IdleThreadNum())
      {      
          Condition_wait(&this->condition);
      }
  }
  Condition_unlock(&this->condition);
  Condition_destroy(&this->condition);

  return;  
} 


int ThreadPool::MoveToIdle(pthread_t tid)  
{
  int tag=0;
  auto busyIter = vecBusyThread.begin();
  for(; busyIter != vecBusyThread.end() ; busyIter++)  
  {  
    if(tid == *busyIter)  
    {  
      vecBusyThread.erase(busyIter); 
      vecIdleThread.push_back(tid); tag=1; 
      break;  
    } 
  } 
  if(busyIter==vecBusyThread.end() && tag==0) 
    cout<<"MoveToIdle Fail:"<<tid<<endl;   
  return 0;  
}  


int ThreadPool::MoveToBusy(pthread_t tid)  
{ 
  int tag=0;
  auto idleIter = vecIdleThread.begin();
  for(; idleIter != vecIdleThread.end() ; idleIter++)  
  {  
    if(tid == *idleIter)  
    {  
      vecIdleThread.erase(idleIter); 
      vecBusyThread.push_back(tid);   
      tag=1;
      break;  
    }  
  } 
  if(idleIter==vecIdleThread.end() && tag==0){ 
    cout<<"MoveToBusy Fail:"<<tid<<endl;
  }
 
  return 0;  
} 
int ThreadPool::DelThread(pthread_t tid){
  int tag=0;
  auto idleIter = vecIdleThread.begin();
  for(; idleIter != vecIdleThread.end() ; idleIter++)  
  {  
    if(tid == *idleIter)  
    {  
      vecIdleThread.erase(idleIter); 
      tag=1;
      break;  
    }  
  } 
  auto busyIter = vecBusyThread.begin();
  for(; busyIter != vecBusyThread.end() ; busyIter++)  
  {  
    if(tid == *busyIter)  
    {  
      vecBusyThread.erase(busyIter); 
      tag=1; 
      break;  
    } 
  } 
  if(idleIter==vecIdleThread.end() && tag==0){ 
    cout<<"DelThread Fail:"<<tid<<endl;
  }
 
  return 0;  
} 

int ThreadPool::IdleThreadNum(){
  return vecIdleThread.size();
}
int ThreadPool::BusyThreadNum(){
  return vecBusyThread.size();
}
int ThreadPool::TaskNum(){
  return queueTaskList.size();
}

