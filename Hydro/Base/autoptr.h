#ifndef AUTOPTR_H
#define AUTOPTR_H
#include "base_globle.h"
//#include <mutex>

namespace SGIS{

template<typename T>
class AutoPtr final{
public:
    AutoPtr() {

    };
    AutoPtr(T*obj,bool onlyRef=false) {
        if(obj==nullptr){
            if (ptr_) Release();
            return;
        }
        if (ptr_) ReleaseObject();
        ptr_ = obj;
        if(count_==nullptr) count_ = new int;
        this->onlyRef=onlyRef;
        *count_ = 1;
    };
    AutoPtr(const AutoPtr& in) {
        if (ptr_) Release();
        ptr_ = in.ptr_;
        count_ = in.count_;
        onlyRef=in.onlyRef;
        AddRef();
    };

    template<typename M>
    AutoPtr(const AutoPtr<M>& in){
        if (ptr_) Release();
        T*oM=dynamic_cast<T*>(in.get());
        if(oM!=nullptr){
            ptr_ = oM;
            count_ = in.countdata();
            onlyRef=in.getOnlyRef();
            AddRef();
        }
        else
           ptr_=nullptr;
    };
    /*
    template<typename M>
    AutoPtr(M*obj,bool onlyRef=false) {
        if (ptr_) ReleaseObject();
        T*oM=dynamic_cast<T*>(obj);
        if(oM!=nullptr){
            ptr_ = oM;
            if(count_==nullptr) count_ = new int;
            *count_ = 1;
            this->onlyRef=onlyRef;
        }
        else
           ptr_=nullptr;
    };*/
    ~AutoPtr() {
        Release();
    };

    bool getOnlyRef() const {
       return onlyRef;
    };

    AutoPtr& operator=(const AutoPtr& in) {
        if (ptr_!=nullptr) Release();
        ptr_ = in.ptr_;
        count_ = in.count_;
        onlyRef=in.onlyRef;
        AddRef();
        return *this;
    };
    bool operator ==(T*other){
        return (ptr_==other);
    };
    bool operator ==(AutoPtr<T>other){
        return (ptr_==other.ptr_);
    };
    bool operator !=(T*other){
        return (ptr_!=other);
    };
    bool operator !=(AutoPtr<T>other){
        return (ptr_!=other.ptr_);
    };

    T&operator*() {
        return *ptr_;
    };
    T* operator->() {
        return ptr_;
    };
    T*get() const{
       return ptr_;
    };
    int user_count(){
        return *count_;
    };
    int*countdata() const{
        return count_;
    };
protected:
    void AddRef() {
        if(count_) ++(*count_);
    }
    void Release() {
        if ((count_!=nullptr) && --(*count_) == 0) {
            if(!onlyRef) delete ptr_;
            delete count_;
            ptr_ = nullptr;
            count_ = nullptr;
        }
    };
    void ReleaseObject(){
        if ((count_!=nullptr) && --(*count_) == 0) {
            delete ptr_;
            ptr_ = nullptr;
        }
    };
protected:
    T* ptr_ = nullptr;
    int* count_ = nullptr;
    bool onlyRef=false;
};

template<typename T>
class AutoArrayPtr final{
public:
    AutoArrayPtr() {

    };

    AutoArrayPtr(T*obj,bool onlyRef=false) {
        if(obj==nullptr){
            if (ptr_) Release();
            return;
        }
        if (ptr_) ReleaseObject();
        ptr_ = obj;
        if(count_==nullptr) count_ = new int;
        this->onlyRef=onlyRef;
        *count_ = 1;
    };
    AutoArrayPtr(const AutoArrayPtr& in) {
        if (ptr_) Release();
        ptr_ = in.ptr_;
        count_ = in.count_;
        onlyRef=in.onlyRef;
        AddRef();
    };
    template<typename M>
    AutoArrayPtr(const AutoArrayPtr<M>& in){
        if (ptr_) Release();
        T*oM=dynamic_cast<T*>(in.get());
        if(oM!=nullptr){
            ptr_ = oM;
            count_ = in.countdata();
            onlyRef=in.getOnlyRef();
            AddRef();
        }
        else
           ptr_=nullptr;
    };
    /*
    template<typename M>
    AutoArrayPtr(M*obj,bool onlyRef=false) {
        if (ptr_) ReleaseObject();
        T*oM=dynamic_cast<T*>(obj);
        if(oM!=nullptr){
            ptr_ = oM;
            if(count_==nullptr) count_ = new int;
            *count_ = 1;
            this->onlyRef=onlyRef;
        }
        else
           ptr_=nullptr;
    };
    */
    ~AutoArrayPtr() {
        Release();
    };

    bool getOnlyRef() const {
       return onlyRef;
    };

    AutoArrayPtr& operator=(const AutoArrayPtr& in) {
        if (ptr_!=nullptr) Release();
        ptr_ = in.ptr_;
        count_ = in.count_;
        onlyRef=in.onlyRef;
        AddRef();
        return *this;
    };
    bool operator ==(T*other){
        return (ptr_==other);
    };
    bool operator ==(AutoArrayPtr<T>other){
        return (ptr_==other.ptr_);
    };
    bool operator !=(T*other){
        return (ptr_!=other);
    };
    bool operator !=(AutoArrayPtr<T>other){
        return (ptr_!=other.ptr_);
    };
    T&operator [](int nIndex){
        return ptr_[nIndex];
    };

    T*get() const{
       return ptr_;
    };
    int user_count(){
        return *count_;
    };
    int*countdata() const{
        return count_;
    };
protected:
    void AddRef() {
        if(count_) ++(*count_);
    }
    void Release() {
        if ((count_!=nullptr) && --(*count_) == 0) {
            if(!onlyRef) delete []ptr_;
            delete count_;
            ptr_ = nullptr;
            count_ = nullptr;
        }
    };
    void ReleaseObject(){
        if ((count_!=nullptr) && --(*count_) == 0) {
            delete []ptr_;
            ptr_ = nullptr;
        }
    };
protected:
    T* ptr_ = nullptr;
    int* count_ = nullptr;
    bool onlyRef=false;
};

}

#endif // AUTOPTR_H
