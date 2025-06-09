#ifndef SEMAPHORA_H
#define SEMAPHORA_H
#include "autoptr.h"
#include <mutex>
#include <condition_variable>
namespace SGIS{

class SGIS_EXPORT Mutex{
public:
    Mutex();
    virtual~Mutex();
    void Lock();
    void Unlock();
protected:
    void*mutex_;
};

class SGIS_EXPORT Semaphora
{
public:
    Semaphora(bool hasSignal,bool autoreset=true);
    virtual~Semaphora();
    void SetSignal();
    void ResetSignal();
    bool HasSignal();
    bool Wait(int miniseconds=0);
protected:
    bool autoreset;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool hasSignal;
};

class SGIS_EXPORT Semaphoras{
public:
    Semaphoras(bool zerofirst=true);
    Semaphoras(const vector<AutoPtr<Semaphora>>&seps,bool zerofirst=true);
    virtual~Semaphoras();
    void Clear();
    void Add(AutoPtr<Semaphora>sep);
    AutoPtr<Semaphora>GetItem(int nIndex);
    int WaitForAny(int miniseconds=0);
    bool WaitForAll(int miniseconds=0);
    int PeepForAny();
protected:
    vector<AutoPtr<Semaphora>>seps;
    bool zerofirst;
    int lastThread;
};

class SGIS_EXPORT SemaphoraHelper{
public:
    static bool Wait(Semaphora*signal,int miniseconds=0);
};

};
#endif // SEMAPHORA_H
