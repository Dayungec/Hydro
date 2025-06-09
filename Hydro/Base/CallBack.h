#ifndef CALLBACK_H
#define CALLBACK_H
#include "base_globle.h"
#include <QString>

namespace SGIS{

class SGIS_EXPORT CallBack{
public:
    CallBack(){};
    virtual~CallBack(){};
    virtual void SendMessage(std::string mess)=0;
};

class SGIS_EXPORT Progress
        :public CallBack{
public:
    Progress(){
        progressText="";
    };
    virtual~Progress(){

    };
    void BeginProgress(string mess){
        this->progressText=progressText;
        SendMessage("开始"+mess+",请稍候...");
        beginProgress();
    };
    virtual void SendProgress(int progress)=0;
    virtual void EndProgress(){
        SendMessage("已完成"+progressText+".");
        endProgress();
    };
protected:
    virtual void beginProgress()=0;
    virtual void endProgress()=0;
protected:
    string progressText;
};

class SGIS_EXPORT CallBackHelper{
public:
    static void SendMessage(CallBack*callBack,string mess){
        if(callBack==nullptr) return;
        callBack->SendMessage(mess);
    };
    static void SendMessageA(CallBack*callBack,string mess){
        if(callBack==nullptr) return;
        callBack->SendMessage(mess);
    };
    static void BeginProgress(CallBack*callBack,string mess){
        if(callBack==nullptr) return;
         Progress*pro=dynamic_cast<Progress*>(callBack);
        if(pro==nullptr)
            callBack->SendMessage("开始"+mess+",请稍候...");
        else
            pro->BeginProgress(mess);
    };
    static void EndProgress(CallBack*callBack){
        if(callBack==nullptr) return;
        Progress*pro=dynamic_cast<Progress*>(callBack);
        if(pro==nullptr)
            callBack->SendMessage("已完成");
        else
            pro->EndProgress();
    };
    static void SendProgress(CallBack*callBack,int progress){
        if(callBack==nullptr) return;
        Progress*pro=dynamic_cast<Progress*>(callBack);
        if(pro==nullptr){
            static int formerpro=0;
            if(formerpro!=progress){
                callBack->SendMessage("已完成"+to_string(progress)+"%");
                formerpro=progress;
            }
        }
        else
            pro->SendProgress(progress);
    };
    static void SendProgress(CallBack*callBack,int value,int total){
        if(callBack==nullptr) return;
        Progress*pro=dynamic_cast<Progress*>(callBack);
        int progress=value*100.0/total;
        if(pro==nullptr){
            static int formerpro=0;
            if(formerpro!=progress){
                callBack->SendMessage("已完成"+to_string(progress)+"%");
                formerpro=progress;
            }
        }
        else
            pro->SendProgress(progress);
    };
};

}

#endif // CALLBACK_H
