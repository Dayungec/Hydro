#include "statishistory.h"

namespace SGIS{

StatisHistory::StatisHistory()
{

}
StatisHistory::~StatisHistory()
{

}
void StatisHistory::AddStatisHist(string key,double StaMin,double StaMax,double StaMean,double StaStand)
{
    int index=keys.Find(key);
    if(index==-1)
    {
        int nPos=keys.Add(key);
        StaMins.insert(begin(StaMins)+nPos,StaMin);
        StaMaxs.insert(begin(StaMaxs)+nPos,StaMax);
        StaMeans.insert(begin(StaMeans)+nPos,StaMean);
        StaStands.insert(begin(StaStands)+nPos,StaStand);
    }
    else
    {
        StaMins[index]=StaMin;
        StaMaxs[index]=StaMax;
        StaMeans[index]=StaMean;
        StaStands[index]=StaStand;
    }
}
bool StatisHistory::RemoveStatis(string key)
{
    int index=keys.Find(key);
    if(index==-1) return false;
    keys.Remove(index);
    StaMins.erase(begin(StaMins)+index);
    StaMaxs.erase(begin(StaMaxs)+index);
    StaMeans.erase(begin(StaMeans)+index);
    StaStands.erase(begin(StaStands)+index);
    return true;
}
bool StatisHistory::GetStatis(string key,double&StaMin,double&StaMax,double&StaMean,double&StaStand)
{
    int index=keys.Find(key);
    if(index==-1) return false;
    StaMin=StaMins[index];
    StaMax=StaMaxs[index];
    StaMean=StaMeans[index];
    StaStand=StaStands[index];
    return true;
}
void StatisHistory::Clear()
{
    StaMins.clear();
    StaMaxs.clear();
    StaMeans.clear();
    StaStands.clear();
    keys.Clear();
}


}
