#include "mcallback.h"
#include "Base/datetime.h"
#include "Base/StringHelper.h"
#include <iostream>

MCallBack::MCallBack(){

}

MCallBack::~MCallBack(){

}

void MCallBack::SendMessage(string mess){
    DateTime dtime;
    string sMess=StringHelper::ToLocalString(dtime.ToFullString(tstFull)+"  "+mess);
    cout<<sMess.c_str()<<endl;
}
