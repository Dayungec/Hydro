#ifndef CLASSFACTORY_H
#define CLASSFACTORY_H
#include "base_globle.h"
#include "autoptr.h"
namespace SGIS{

typedef void* (*func_ptr)();
typedef std::map<const string, func_ptr> create_obj_map;

#define REGISTER(ClassName) \
    class Register##ClassName \
    { \
    public: \
        static void* instance() \
        { \
            return new ClassName; \
        } \
    private: \
        static const Register _staticRegister; \
    }; \
    const Register Register##ClassName::_staticRegister(#ClassName, Register##ClassName::instance);\

class SGIS_EXPORT SGISSingleton
{
public:
    template<class T>
    static T& instance()
    {
        static T _instance;
        return _instance;
    }
};

class SGIS_EXPORT Object{
public:
    Object();
    virtual~Object();
    virtual string GetTypeName()=0;
};

class SGIS_EXPORT ClassFactory
{
public:
   static AutoPtr<Object>getInstance(const string &className);
   static void registerClass(const string &className, func_ptr fp);
   static vector<string>getClassNames();
   template<typename T>
   static vector<AutoPtr<T>>getObjects(){
       vector<string>clses=getClassNames();
       vector<AutoPtr<T>>obs;
       for(int k=0;k<clses.size();k++){
           AutoPtr<T>ob=getInstance(clses[k]);
           if(ob!=nullptr) obs.push_back(ob);
       }
       return obs;
   };
};

class SGIS_EXPORT Register
{
public:
    Register(const char* className, func_ptr fp);
};



}
#endif // CLASSFACTORY_H
