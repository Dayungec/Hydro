#include "threadpool.h"
#include "file.h"

namespace SGIS{

ThreadPool::ThreadPool(unsigned short size)
{
    AddThread(size);
}
ThreadPool::~ThreadPool()
{
    _run=false;
    _task_cv.notify_all(); // 唤醒所有线程执行
    for (thread& thread : _pool) {
        //thread.detach(); // 让线程“自生自灭”
        if(thread.joinable())
            thread.join(); // 等待任务结束， 前提：线程一定会执行完
    }
}

UINT ThreadPool::GetNumberOfProcessors(){
    return std::thread::hardware_concurrency();
}


void ThreadPool::AddThread(unsigned short size)
{
    for (int k=0;k<size;k++)
    {   //增加线程数量
        _pool.emplace_back( [this]{ //工作线程函数
            while (_run)
            {
                Task task; // 获取一个待执行的 task
                {
                    // unique_lock 相比 lock_guard 的好处是：可以随时 unlock() 和 lock()
                    unique_lock<mutex> lock{ _lock };
                    _task_cv.wait(lock, [this]{
                        return (!_run || !_tasks.empty());
                    }); // wait 直到有 task
                    if ((!_run && _tasks.empty())){
                        return;
                    }
                    task = move(_tasks.front()); // 按先进先出从队列取一个 task
                    _tasks.pop();
                }
                _idlThrNum--;
                task();//执行任务
                _idlThrNum++;
            }
        });
        _idlThrNum++;
    }
}


int ThreadPool::GetIdlCount()
{
    return _idlThrNum;
}
//线程数量
int ThreadPool::GetThreadCount()
{
    return _pool.size();
}

TaskPool::TaskPool(int threadCount):
    pool(threadCount){
    taskFinished=true;
}
TaskPool::~TaskPool(){

}

void TaskPool::LabelTaskFinished(){
    mutex.Lock();
    this->taskFinished=true;
    mutex.Unlock();
}

void TaskPool::ClearTasksAndWait(){
    LabelTaskFinished();
    WaitAll();
}

void TaskPool::WaitAll(){
    for (auto && result : results){
        result.get();
    }
    results.clear();
}
void TaskPool::ExecuteTask(void*attachObject,string taskName,int taskCount,EVENTFUN fun){
    //ClearTasksAndWait();
    //this->attachObject=attachObject;
    //this->taskCount=taskCount;
    //this->taskName=taskName;
    //this->fun=fun;
    mutex.Lock();
    this->taskFinished=false;
    mutex.Unlock();
    //TaskPool*pFather=this;
    std::vector< std::future<void> > results;
    for(int k=0;k<taskCount;k++){
        if(taskFinished) break;
        results.emplace_back(
        pool.Commit([this,k,attachObject,taskName,fun]{
            Event event(attachObject,EventArgs({
               {"taskname",taskName},
               {"taskid",k}
            }));
            this->mutex.Lock();
            bool taskFinished=this->taskFinished;
            this->mutex.Unlock();
            if(!taskFinished) fun(event);
        }));
    }
    for (auto && result : results){
        result.get();
    }
}

void TaskPool::ExecuteSingleTask(void*attachObject,EVENTFUN fun){
    mutex.Lock();
    this->taskFinished=false;
    mutex.Unlock();
    std::future<void>result;
    result=pool.Commit([this,attachObject,fun]{
        Event event(attachObject,EventArgs());
        this->mutex.Lock();
        bool taskFinished=this->taskFinished;
        this->mutex.Unlock();
        if(!taskFinished) fun(event);
    });
    result.get();
}

void TaskPool::AddTasks(void*attachObject,string taskName,int taskCount,EVENTFUN fun){
    //ClearTasksAndWait();
    //WaitAll();
    mutex.Lock();
    this->taskFinished=false;
    mutex.Unlock();
    for(int k=0;k<taskCount;k++){
        if(taskFinished) break;
        results.emplace_back(
        pool.Commit([this,k,attachObject,taskName,fun]{
            Event event(attachObject,EventArgs({
               {"taskname",taskName},
               {"taskid",k}
            }));
            if(!this->taskFinished)
                fun(event);
            else
                return;
        }));
    }
}

//string TaskPool::GetTaskName(){
    //return taskName;
//}

ResourceIndex::ResourceIndex(ResourcePool*pPool,double waitMiniseconds){
    this->pPool=pPool;
    resourceIndex=pPool->LockResource(waitMiniseconds);
}

ResourceIndex::~ResourceIndex(){
    if(resourceIndex>=0) pPool->ReleaseResource(resourceIndex);
}

int ResourceIndex::GetResourceId(){
    return resourceIndex;
}

ResourcePool::ResourcePool(int resourceCount){
    this->resourceCount=resourceCount;
    _pool.resize(resourceCount);
    for(int k=0;k<resourceCount;k++) _pool[k]=false;
    curPin=0;
}
ResourcePool::~ResourcePool(){

}
int ResourcePool::LockResource(double waitMiniseconds){
    //int id=-1;
    if(waitMiniseconds<=0){
        while(true){
            mutex.Lock();
            for(int k=0;k<resourceCount;k++){
                int idx=k+curPin;
                if(idx>=resourceCount) idx-=resourceCount;
                if(!_pool[idx]){
                    //id=idx;
                    _pool[idx]=true;
                    curPin=idx+1;
                    if(curPin>=resourceCount) curPin-=resourceCount;
                    mutex.Unlock();
                    return idx;
                }
            }
            mutex.Unlock();
        }
    }
    else{
        DateTime curTime;
        while(true){
            mutex.Lock();
            for(int k=0;k<resourceCount;k++){
                int idx=k+curPin;
                if(idx>=resourceCount) idx-=resourceCount;
                if(!_pool[idx]){
                    //id=idx;
                    _pool[idx]=true;
                    curPin=idx+1;
                    if(curPin>=resourceCount) curPin-=resourceCount;
                    mutex.Unlock();
                    return idx;
                }
            }
            mutex.Unlock();
            DateTime time;
            double seconds=(time-curTime).TotalSeconds()*1000.0;
            if(seconds>waitMiniseconds) break;
        }
    }
    return -1;
}


AutoPtr<ResourceIndex>ResourcePool::LockResourceEx(double waitMiniseconds){
   ResourceIndex*ri=new ResourceIndex(this,waitMiniseconds);
   return ri;
}

void ResourcePool::ReleaseResource(int id){
    mutex.Lock();
    _pool[id]=false;
    mutex.Unlock();
}

int ResourcePool::GetResourceCount(){
    return resourceCount;
}

BackTaskManager::BackTaskManager():
    pool(1){
    taskFinished=true;
    signal=new Semaphora(false,false);
}
BackTaskManager::~BackTaskManager(){
    StopAndWait();
}
bool BackTaskManager::GetTaskRunning(){
    mutex.Lock();
    bool runing=(!taskFinished);
    mutex.Unlock();
    return runing;
}

bool BackTaskManager::GetTaskRunningWithoutLock(){
    return (!taskFinished);
}

bool BackTaskManager::RunTask(void*attachObject,string taskName,EVENTFUN fun){
   if(GetTaskRunning()) return false;
   this->attachObject=attachObject;
   this->taskCount=taskCount;
   this->taskName=taskName;
   this->fun=fun;
   taskFinished=false;
   BackTaskManager*pFather=this;
   int k=0;
   signal->SetSignal();
   results.emplace_back(
     pool.Commit([this,k,attachObject,pFather]{
       Event event(attachObject,EventArgs({
           {"taskname",pFather->taskName},
           {"signal",signal.get()}
       }));
       pFather->mutex.Lock();
       pFather->TaskBegin.RaiseEvent(pFather);
       signal->ResetSignal();
       pFather->mutex.Unlock();
       pFather->fun(event);
       pFather->mutex.Lock();
       signal->SetSignal();
       pFather->taskFinished=true;
       pFather->TaskFinished.RaiseEvent(pFather);
       pFather->mutex.Unlock();
   }));
   return true;
}

string BackTaskManager::GetTaskName(){
    return taskName;
}


AutoPtr<Semaphora>BackTaskManager::GetSignal(){
    return signal;
}


void BackTaskManager::StopAndWait(){
    mutex.Lock();
    this->taskFinished=true;
    signal->SetSignal();
    mutex.Unlock();
    for (auto && result : results){
        result.get();
    }
    results.clear();
}

void BackTaskManager::SetTaskFinished(){
    mutex.Lock();
    this->taskFinished=true;
    signal->SetSignal();
    mutex.Unlock();
}

}
