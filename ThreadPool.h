#ifndef __CTHREAD  
#define __CTHREAD  
#include <queue>  
#include <string>  
#include <vector>
#include <pthread.h>  
#include "Condition.h"     
using namespace std;  
 
/** 
执行任务的类，设置任务数据并执行 
**/  
class Task  
{  
protected:  
  string m_TaskName;  //任务的名称  
  void* m_ptrData;    //要执行的任务的具体数据(函数指针)  
public:  
  Task(){}  
  Task(string taskName);  
  virtual int Run()= 0;  
  void SetData(void* data);    //设置任务数据  
};  

/** 
线程池 
**/  
class ThreadPool  
{  
private:  
  queue<Task*> queueTaskList;         //任务列表  
  int maxThreadNum;                            //线程池中启动的线程数             
  vector<pthread_t> vecIdleThread;   //当前空闲的线程集合  
  vector<pthread_t> vecBusyThread;   //当前正在执行的线程集合 
  Condition_t condition; 
  bool quit;
 
protected:  
  static void* ThreadFunc(void*); //执行任务的任务线程函数 
  int MoveToIdle(pthread_t tid); //线程执行结束后，把自己放入到空闲线程中  
  int MoveToBusy(pthread_t tid); //移入到忙碌线程中去  
  int DelThread(pthread_t tid);
public:  
  int Create();          //创建线程池  
  ThreadPool(int threadNum);  
  int AddTask(Task *task);      //把任务添加到线程池中 
  int TaskNum();
  int IdleThreadNum();
  int BusyThreadNum();
  int Signal();   //通知线程执行任务
  void DestroyAll();
  int runpos;
  
}; 

 
#endif  