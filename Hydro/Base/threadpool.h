#ifndef THREADPOOL_H
#define THREADPOOL_H
#include "autoptr.h"
#include "semaphora.h"
#include "eventhelper.h"
#include <thread>

#include <vector>
#include <queue>
#include <atomic>
#include <future>
//#include <condition_variable>
//#include <thread>
#include <functional>
#include <queue>
#include <stdexcept>

namespace SGIS{

class SGIS_EXPORT ThreadPool
{
public:
    ThreadPool(unsigned short size = 4);
    virtual~ThreadPool();
public:
    template<class F, class... Args>
    auto Commit(F&& f, Args&&... args) ->future<decltype(f(args...))>
    {
        if (!_run)
            throw runtime_error("commit on ThreadPool is stopped.");

        using RetType = decltype(f(args...));
        auto task = make_shared<packaged_task<RetType()>>(
            bind(forward<F>(f), forward<Args>(args)...)
        );
        future<RetType> future = task->get_future();
        {
            lock_guard<mutex> lock{ _lock };
            _tasks.emplace([task](){
                (*task)();
            });
        }
        _task_cv.notify_one();
        return future;
    };
    int GetIdlCount();
    int GetThreadCount();
    static UINT GetNumberOfProcessors();
private:
    void AddThread(unsigned short size);
public:
    using Task = function<void()>;
    vector<thread> _pool;
    queue<Task> _tasks;
    mutex _lock;
    condition_variable _task_cv;
    atomic<bool> _run{ true };
    atomic<int>  _idlThrNum{ 0 };
};

class SGIS_EXPORT TaskPool
{
public:
    TaskPool(int threadCount);
    virtual~TaskPool();
    void AddTasks(void*attachObject,string taskName,int taskCount,EVENTFUN fun);
    void ExecuteTask(void*attachObject,string taskName,int taskCount,EVENTFUN fun);
    void ExecuteSingleTask(void*attachObject,EVENTFUN fun);
    //string GetTaskName();
    void LabelTaskFinished();
    void ClearTasksAndWait();
protected:
    void WaitAll();
protected:
    int threadCount;
    //int taskCount;
    //string taskName;
    //void*attachObject;
    //EVENTFUN fun;
    ThreadPool pool;
    bool taskFinished;
    Mutex mutex;
    std::vector< std::future<void> > results;
};

class ResourcePool;
class SGIS_EXPORT ResourceIndex{
protected:
    ResourceIndex(ResourcePool*pPool,double waitMiniseconds=0);
public:
    virtual~ResourceIndex();
    int GetResourceId();
protected:
    friend class ResourcePool;
    ResourcePool*pPool;
    int resourceIndex;
};


class SGIS_EXPORT ResourcePool{
public:
    ResourcePool(int resourceCount);
    virtual~ResourcePool();
    int LockResource(double waitMiniseconds=0);
    void ReleaseResource(int id);
    int GetResourceCount();
    AutoPtr<ResourceIndex>LockResourceEx(double waitMiniseconds=0);
protected:
    int resourceCount;
    vector<bool>_pool;
    Mutex mutex;
    int curPin;
};

template<typename T>
class SGIS_EXPORT ResourceObjects;

template<typename T>
class SGIS_EXPORT ResourceObject{
protected:
    ResourceObject(ResourceObjects<T>*objs,double miniSeconds=0){
        this->objs=objs;
        resourceIndex=this->objs->pool->LockResourceEx(miniSeconds);
    };
public:
    virtual~ResourceObject(){

    };
    AutoPtr<T> object(){
        int rid=resourceIndex->GetResourceId();
        if(rid<0) return nullptr;
        return objs->objs[rid];
    };
    int GetResourceId(){
        return resourceIndex->GetResourceId();
    };
protected:
    friend class ResourceObjects<T>;
    ResourceObjects<T>*objs;
    AutoPtr<ResourceIndex>resourceIndex;
};

template<typename T>
class SGIS_EXPORT ResourceObjects{
public:
    ResourceObjects(int resourceCount){
        pool=new ResourcePool(resourceCount);
        objs.resize(resourceCount);
        for(int k=0;k<resourceCount;k++){
            objs[k]=new T();
        }
    };
    ResourceObjects(int resourceCount,vector<AutoPtr<T>>&objects){
        pool=new ResourcePool(resourceCount);
        objs.resize(resourceCount);
        for(int k=0;k<resourceCount;k++){
            objs[k]=objects[k];
        }
    };
    virtual~ResourceObjects(){

    };
    AutoPtr<ResourceObject<T>>acquire(double miniSeconds=0){
        AutoPtr<ResourceObject<T>>obj=new ResourceObject<T>(this,miniSeconds);
        if(obj->object()==nullptr) return nullptr;
        return obj;
    };
    AutoPtr<T>getByIndex(int nIndex){
        return objs[nIndex];
    };
protected:
    friend class ResourceObject<T>;
    AutoPtr<ResourcePool>pool;
    vector<AutoPtr<T>>objs;
};


class SGIS_EXPORT BackTaskManager{
public:
    BackTaskManager();
    virtual~BackTaskManager();
    bool GetTaskRunning();
    bool GetTaskRunningWithoutLock();
    bool RunTask(void*attachObject,string taskName,EVENTFUN fun);
    void StopAndWait();
    void SetTaskFinished();
    string GetTaskName();
    AutoPtr<Semaphora>GetSignal();
public:
    Event TaskBegin;
    Event TaskFinished;
protected:
    int threadCount;
    int taskCount;
    string taskName;
    void*attachObject;
    EVENTFUN fun;
    ThreadPool pool;
    bool taskFinished;
    Mutex mutex;
    AutoPtr<Semaphora>signal;
    std::vector< std::future<void> > results;
};

};
#endif // THREADPOOL_H
