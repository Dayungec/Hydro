#include "classfactory.h"

namespace SGIS{

Object::Object(){

}

Object::~Object(){

}

AutoPtr<Object>ClassFactory::getInstance(const string &className)
{
    create_obj_map::iterator iter = SGISSingleton::instance<create_obj_map>().begin();
    create_obj_map::iterator iterEnd = SGISSingleton::instance<create_obj_map>().end();
    while (iter != iterEnd)
    {
        if (iter->first == className)
        {
            return AutoPtr<Object>((Object*)iter->second(),true);
        }
        iter++;
    }
    return nullptr;
}

void ClassFactory::registerClass(const string &className, func_ptr fp)
{
    SGISSingleton::instance<create_obj_map>().insert(std::make_pair(className, fp));
}

vector<string>ClassFactory::getClassNames(){
    create_obj_map::iterator iter = SGISSingleton::instance<create_obj_map>().begin();
    create_obj_map::iterator iterEnd = SGISSingleton::instance<create_obj_map>().end();
    vector<string>clsNames;
    while (iter != iterEnd)
    {
        clsNames.push_back(iter->first);
        iter++;
    }
    return clsNames;
}

Register::Register(const char* className, func_ptr fp)
{
    ClassFactory::registerClass(className, fp);
}



}
