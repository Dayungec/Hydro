#ifndef MCALLBACK_H
#define MCALLBACK_H

#include "Base/CallBack.h"

using namespace SGIS;
class MCallBack :public CallBack
{
public:
    MCallBack();
    virtual~MCallBack();
    void SendMessage(string mess);
};

#endif // MCALLBACK_H
