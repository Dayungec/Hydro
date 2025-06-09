#ifndef TASK_H
#define TASK_H

#include "Base/classfactory.h"
#include "Base/CallBack.h"

using namespace SGIS;

class Task
    :public Object
{
public:
    Task();
    virtual~Task();
    virtual string GetTaskName()=0;
    virtual string GetTypeName(){
        return GetTaskName()+"Task";
    };
    static vector<AutoPtr<Task>>CreateStockTasks();
    static AutoPtr<Task>CreateTask(string taskName);
    static void SendCmds(CallBack*callBack);
public:
    virtual bool ReadParameters(vector<string>argstrs, CallBack*callBack) {
        return true;
    };
    virtual void SendHelpString(CallBack*callBack) {
        callBack->SendMessage(GetTaskName());
    };
    virtual void Execute(CallBack*callBack)=0;
protected:
    bool GetBoolValue(string arg);
    void Sleep(int miniseconds);
protected:
    string id;
};

class FillDemTask:
    public Task{
public:
    FillDemTask();
    virtual~FillDemTask();
    string GetTypeName();
    string GetTaskName();
    bool ReadParameters(vector<string>argstrs, CallBack*callBack);
    void Execute(CallBack*callBack);
    void SendHelpString(CallBack*callBack);
protected:
    string demPath;
    string savingPath;
};

class FlowDirectionTask:
    public Task{
public:
    FlowDirectionTask();
    virtual~FlowDirectionTask();
    string GetTypeName();
    string GetTaskName();
    bool ReadParameters(vector<string>argstrs, CallBack*callBack);
    void Execute(CallBack*callBack);
    void SendHelpString(CallBack*callBack);
protected:
    string demPath;
    string savingPath;
};

class FlowAccumulationTask:
                    public Task{
public:
    FlowAccumulationTask();
    virtual~FlowAccumulationTask();
    string GetTypeName();
    string GetTaskName();
    bool ReadParameters(vector<string>argstrs, CallBack*callBack);
    void Execute(CallBack*callBack);
    void SendHelpString(CallBack*callBack);
protected:
    string demPath;
    string savingPath;
};

class SinkTask:
    public Task{
public:
    SinkTask();
    virtual~SinkTask();
    string GetTypeName();
    string GetTaskName();
    bool ReadParameters(vector<string>argstrs, CallBack*callBack);
    void Execute(CallBack*callBack);
    void SendHelpString(CallBack*callBack);
protected:
    string demPath;
    int lakeMinRadius;
    string savingPath;
};


class SpillLineTask:
    public Task{
public:
    SpillLineTask();
    virtual~SpillLineTask();
    string GetTypeName();
    string GetTaskName();
    bool ReadParameters(vector<string>argstrs, CallBack*callBack);
    void Execute(CallBack*callBack);
    void SendHelpString(CallBack*callBack);
protected:
    string demPath;
    string filldemPath;
    string flowPath;
    string accuPath;
    int lakeMinRadius;
    string savingPath;
};


class DigTask:
    public Task{
public:
    DigTask();
    virtual~DigTask();
    string GetTypeName();
    string GetTaskName();
    bool ReadParameters(vector<string>argstrs, CallBack*callBack);
    void Execute(CallBack*callBack);
    void SendHelpString(CallBack*callBack);
protected:
    string demPath;
    string spillLinePath;
    string savingPath;
};

class SpillLineDigTask:
    public Task{
public:
    SpillLineDigTask();
    virtual~SpillLineDigTask();
    string GetTypeName();
    string GetTaskName();
    bool ReadParameters(vector<string>argstrs, CallBack*callBack);
    void Execute(CallBack*callBack);
    void SendHelpString(CallBack*callBack);
protected:
    string demPath;
    int lakeMinRadius;
    int iters;
    string savingPath;
};



#endif // TASK_H
