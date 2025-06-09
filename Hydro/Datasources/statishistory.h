#ifndef STATISHISTORY_H
#define STATISHISTORY_H

#include "Base/autoptr.h"
#include "Base/BaseArray.h"

namespace SGIS{

class SGIS_EXPORT StatisHistory
{
public:
    StatisHistory();
    virtual~StatisHistory();
    void AddStatisHist(string key,double StaMin,double StaMax,double StaMean,double StaStand);
    bool RemoveStatis(string key);
    bool GetStatis(string key,double&StaMin,double&StaMax,double&StaMean,double&StaStand);
    void Clear();
protected:
    vector<double>StaMins;
    vector<double>StaMaxs;
    vector<double>StaMeans;
    vector<double>StaStands;
    OrderArray<string>keys;
};

}

#endif // STATISHISTORY_H
