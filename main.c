#include "Threadpool.h"
#include <unistd.h>
#include <stdlib.h>
#include <iostream> 

class WorkTask: public Task  
{  
public:  
  WorkTask() {}  
  int Run()  
  {  
    cout << (char*)this->m_ptrData << endl;  
    sleep(1);  
    return 0;  
  }  
};

//测试代码
int main(void)
{
  const long long N=100;

  ThreadPool tp(10);
  //初始化线程池，最多三个线程
  int i;
  //创建十个任务
  for(i=0; i < N; i++)
  {
      WorkTask *taskObj = new WorkTask; 
      char *szTmp = new char[30];
      sprintf(szTmp,"this is the %d thread running, Success",i);
      taskObj->SetData((void*)szTmp);
      tp.AddTask(taskObj);   
  }
  tp.DestroyAll();
  return 0;
}