#include "semaphora.h"

#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_types.hpp>
#include <boost/thread.hpp>

namespace SGIS{

Mutex::Mutex(){
    boost::mutex*mutex=new boost::mutex();
    mutex_=mutex;
}
Mutex::~Mutex(){
    boost::mutex*mutex=(boost::mutex*)mutex_;
    delete mutex;
}
void Mutex::Lock(){
    ((boost::mutex*)mutex_)->lock();
}
void Mutex::Unlock(){
    ((boost::mutex*)mutex_)->unlock();
}

Semaphora::Semaphora(bool hasSignal,bool autoreset)
{
    this->hasSignal=hasSignal;
    this->autoreset=autoreset;
}
Semaphora::~Semaphora()
{

}

void Semaphora::SetSignal(){
    std::unique_lock<std::mutex> lock(mutex_);
    hasSignal=true;
    if(autoreset)
       cv_.notify_one();
    else
       cv_.notify_all();
}

void Semaphora::ResetSignal(){
    std::unique_lock<std::mutex> lock(mutex_);
    hasSignal=false;
}

bool Semaphora::HasSignal(){
    return hasSignal;
}

bool Semaphora::Wait(int miniseconds){
    if(miniseconds==0){
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [=] { return hasSignal; });
        if(autoreset){
            hasSignal=false;
        }
        return true;
    }
    else{
        std::unique_lock<std::mutex> lock(mutex_);
        if(cv_.wait_for(lock,std::chrono::milliseconds(miniseconds), [=] { return hasSignal; })){
            if(autoreset){
                hasSignal=false;
            }
            return true;
        }
        return false;
    }
}

Semaphoras::Semaphoras(bool zerofirst){
    this->zerofirst=zerofirst;
    lastThread=0;
}
Semaphoras::Semaphoras(const vector<AutoPtr<Semaphora>>&seps,bool zerofirst){
    this->seps.resize(seps.size());
    for(int k=0;k<seps.size();k++){
        this->seps[k]=seps[k];
    }
    this->zerofirst=zerofirst;
    lastThread=0;
}
Semaphoras::~Semaphoras(){

}

void Semaphoras::Clear(){
    seps.clear();
}

void Semaphoras::Add(AutoPtr<Semaphora>sep){
    seps.push_back(sep);
}

AutoPtr<Semaphora>Semaphoras::GetItem(int nIndex){
    return seps[nIndex];
}

int Semaphoras::PeepForAny(){
    if(zerofirst){
        if(seps[0]->HasSignal()){
            return 0;
        }
        for(int k=0;k<seps.size()-1;k++){
            int npos=k+lastThread;
            if(npos>=seps.size()-1){
                npos-=seps.size()-1;
            }
            if(seps[npos+1]->HasSignal()){
                lastThread=npos+1;
                if(lastThread>=seps.size()-1) lastThread=0;
                return npos+1;
            }
        }
    }
    else{
        for(int k=0;k<seps.size();k++){
            int npos=k+lastThread;
            if(npos>=seps.size()){
                npos-=seps.size();
            }
            if(seps[npos]->HasSignal()){
                lastThread=npos+1;
                if(lastThread>=seps.size()) lastThread=0;
                return npos;
            }
        }
    }
    return -1;
    /*
    if(zerofirst){
        if(seps[0]->HasSignal()){
            return 0;
        }
        if(seps[0]->Wait(1)){
            return 0;
        }
        for(int k=0;k<seps.size()-1;k++){
            int npos=k+lastThread;
            if(npos>=seps.size()-1){
                npos-=seps.size()-1;
            }
            if(seps[npos+1]->Wait(1)){
                lastThread=npos+1;
                if(lastThread>=seps.size()-1) lastThread=0;
                return npos+1;
            }
        }
    }
    else{
        for(int k=0;k<seps.size();k++){
            int npos=k+lastThread;
            if(npos>=seps.size()){
                npos-=seps.size();
            }
            if(seps[npos]->Wait(1)){
                lastThread=npos+1;
                if(lastThread>=seps.size()) lastThread=0;
                return npos;
            }
        }
    }
    return -1;
     */
}

int Semaphoras::WaitForAny(int miniseconds){
    if(miniseconds==0){
        while (true)
        {
            if(zerofirst){
                if(seps[0]->Wait(1)){
                    return 0;
                }
                for(int k=0;k<seps.size()-1;k++){
                    int npos=k+lastThread;
                    if(npos>=seps.size()-1){
                        npos-=seps.size()-1;
                    }
                    if(seps[npos+1]->Wait(1)){
                        lastThread=npos+1;
                        if(lastThread>=seps.size()-1) lastThread=0;
                        return npos+1;
                    }
                }
            }
            else{
                for(int k=0;k<seps.size();k++){
                    int npos=k+lastThread;
                    if(npos>=seps.size()){
                        npos-=seps.size();
                    }
                    if(seps[npos]->Wait(1)){
                        lastThread=npos+1;
                        if(lastThread>=seps.size()) lastThread=0;
                        return npos;
                    }
                }
            }
        }
        return -1;
    }
    else{
        boost::posix_time::ptime initialTime = boost::posix_time::microsec_clock::local_time();
        while (true)
        {
            if(zerofirst){
                if(seps[0]->Wait(1)){
                    return 0;
                }
                for(int k=0;k<seps.size()-1;k++){
                    int npos=k+lastThread;
                    if(npos>=seps.size()-1){
                        npos-=seps.size()-1;
                    }
                    if(seps[npos+1]->Wait(1)){
                        lastThread=npos+1;
                        if(lastThread>=seps.size()-1) lastThread=0;
                        return npos+1;
                    }
                }
            }
            else{
                for(int k=0;k<seps.size();k++){
                    int npos=k+lastThread;
                    if(npos>=seps.size()){
                        npos-=seps.size();
                    }
                    if(seps[npos]->Wait(1)){
                        lastThread=npos+1;
                        if(lastThread>=seps.size()) lastThread=0;
                        return npos;
                    }
                }
            }
            boost::posix_time::ptime currentTime = boost::posix_time::microsec_clock::local_time();
            boost::posix_time::time_duration duration = currentTime - initialTime;
            if(duration.ticks()/1000.0>=miniseconds) return -1;
        }
        return -1;
    }
}

bool Semaphoras::WaitForAll(int miniseconds){
    if(miniseconds==0){
        //boost::unique_lock<boost::mutex> lock(*(boost::mutex*)mutex_);
        while (true)
        {
            bool allWaited=true;
            for(int k=0;k<seps.size();k++){
                if(!seps[k]->Wait(1)){
                    allWaited=false;
                }
            }
            if(allWaited) return true;
            //((boost::condition_variable*)condition_)->wait(lock);
        }
        return true;
    }
    else{
        boost::posix_time::ptime initialTime = boost::posix_time::microsec_clock::local_time();
        while (true)
        {
            bool allWaited=true;
            for(int k=0;k<seps.size();k++){
                if(!seps[k]->Wait(1)){
                    allWaited=false;
                }
            }
            if(allWaited) return true;
            boost::posix_time::ptime currentTime = boost::posix_time::microsec_clock::local_time();
            boost::posix_time::time_duration duration = currentTime - initialTime;
            if(duration.ticks()/1000.0>=miniseconds) return false;
        }
        return false;
    }
}

bool SemaphoraHelper::Wait(Semaphora*signal,int miniseconds){
    if(signal==nullptr) return false;
    return signal->Wait(miniseconds);
}


}
