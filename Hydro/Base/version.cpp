#include "version.h"

namespace SGIS{
VersionManager::VersionManager()
{
    CurrentPin=-1;
    HistoryMaxItems=100;
}

VersionManager::~VersionManager(){
    els.clear();
}

int VersionManager::GetVersionCount(){
    return els.size();
}

AutoPtr<Version>VersionManager::GetVersionItem(int nIndex){
    return els[nIndex];
}

int VersionManager::GetHistoryMaxItems(){
    return HistoryMaxItems;
}

void VersionManager::SetHistoryMaxItems(int maxItems){
    HistoryMaxItems=maxItems;
    if(HistoryMaxItems<2) HistoryMaxItems=2;
}

void VersionManager::RegisterVersion(AutoPtr<Version>item){
    int Size=els.size();
    for(int k=Size-1;k>CurrentPin;k--)
    {
        els[k]->ClipForewardAllRedos();
        els.erase(begin(els)+k);
    }
    if(CurrentPin>=HistoryMaxItems-1)
    {
        els.erase(begin(els));
        CurrentPin--;
    }
    els.push_back(item);
    vector<AutoPtr<Version>>regs;
    vector<LONG>times;
    vector<LONG>indexes;
    Size=els.size();
    for(int k=0;k<Size;k++)
    {
        int index=-1;
        AutoPtr<Version>oreg=els[k];
        for(int j=regs.size()-1;j>=0;j--)
        {
            AutoPtr<Version>reg=regs[j];
            if(reg==oreg)
            {
                index=j;
                break;
            }
        }
        if(index>=0)
        {
            times[index]=times[index]+1;
            indexes.push_back(index);
        }
        else
        {
            regs.push_back(oreg);
            times.push_back(1);
            indexes.push_back(regs.size()-1);
        }
    }
    for(int k=regs.size()-1;k>=0;k--)
    {
        AutoPtr<Version>reg=regs[k];
        LONG time=reg->GetUsableUndoTimes();
        LONG time1=reg->GetUsableRedoTimes();
        int dif=time+time1-times[k];
        if(dif>0)
        {
            for(int j=dif;j>0;j--)
            {
                reg->RemoveOneBackwardUndo();
            }
        }
    }
    vector<LONG>nums;
    for(int k=regs.size()-1;k>=0;k--) nums.push_back(0);
    for(int k=Size-1;k>=0;k--)
    {
        AutoPtr<Version>reg=els[k];
        LONG Count=times[indexes[k]];
        int num=nums[indexes[k]];
        nums[indexes[k]]=num+1;
        LONG time=reg->GetUsableUndoTimes();
        LONG time1=reg->GetUsableRedoTimes();
        time=time+time1+1;
        if(num+1>=time)
        {
            els.erase(begin(els)+k);
            times[indexes[k]]=times[indexes[k]]-1;
            nums[indexes[k]]=num;
            indexes.erase(begin(indexes)+k);
            if(CurrentPin>=k) CurrentPin--;
        }
    }
    if(els.size()>0)
        CurrentPin++;
    if(CurrentPin>=els.size())
        CurrentPin=els.size()-1;
    VersionChanged.RaiseEvent(this,EventArgs({{"version",item.get()}}));
}

bool VersionManager::FindOldVersion(AutoPtr<Version>version){
    for(int k=els.size()-1;k>=0;k--)
    {
        if(version==els[k]) return true;
    }
    return false;
}

void VersionManager::EmptyVersions(){
    els.clear();
    CurrentPin=-1;
}
void VersionManager::RemoveVersion(AutoPtr<Version>pVersion){
    for(int k=els.size()-1;k>=0;k--)
    {
        if(els[k]==pVersion)
        {
            els.erase(begin(els)+k);
            if(CurrentPin>=k) CurrentPin--;
        }
    }
}

int VersionManager::GetUsableUndoTimes(){
    int fromPos=CurrentPin;
    for(int k=fromPos;k>=0;k--)
    {
        LONG Times=els[k]->GetUsableUndoTimes();
        if(Times>=0)
        {
            break;
        }
        else
        {
            els.erase(begin(els)+k);
            CurrentPin--;
        }
    }
    return CurrentPin+1;
}
int VersionManager::GetUsableRedoTimes(){
    int fromPos=CurrentPin;
    for(int k=fromPos;k<els.size()-1;k++)
    {
        LONG Times=els[CurrentPin+1]->GetUsableRedoTimes();
        if(Times>0)
        {
            break;
        }
        else
        {
            els.erase(begin(els)+CurrentPin+1);
        }
    }
    return els.size()-1-CurrentPin;
}

bool VersionManager::UndoEdit(){
    if(CurrentPin<0)
        return false;
    else
    {
        int fromPos=CurrentPin;
        for(int k=fromPos;k>=0;k--)
        {
            LONG Times=els[k]->GetUsableUndoTimes();
            if(Times>=0)
            {
                CurrentPin--;
                if (els.size() == 0) CurrentPin = -1;
                els[k]->UndoEdit();
                VersionChanged.RaiseEvent(this,EventArgs({{"version",els[k].get()}}));
                return true;
            }
            else
            {
               els.erase(begin(els)+k);
               CurrentPin--;
            }
        }
        return false;
    }
}
bool VersionManager::RedoEdit(){
    if(CurrentPin+1>=els.size())
        return false;
    else
    {
        int fromPos=CurrentPin;
        for(int k=fromPos+1;k<els.size();k++)
        {
            LONG Times=els[CurrentPin+1]->GetUsableRedoTimes();
            if(Times>0)
            {
                int redoPin=CurrentPin+1;
                if(els.size()>0) CurrentPin++;
                els[redoPin]->RedoEdit();
                VersionChanged.RaiseEvent(this,EventArgs({{"version",els[k].get()}}));
                return true;
            }
            else
            {
                els.erase(begin(els)+CurrentPin+1);
            }
        }
        return false;
    }
}

string VersionManager::GetCurrentUndoName(){
    if((CurrentPin<0)&&(CurrentPin>=els.size())) return "";
    int fromPos=CurrentPin;
    for(int k=fromPos;k>=0;k--)
    {
        LONG Times=els[k]->GetUsableUndoTimes();
        if(Times>=0)
        {
            break;
        }
        else
        {
            els.erase(begin(els)+k);
            CurrentPin--;
        }
    }
    if((CurrentPin>=0)&&(CurrentPin<els.size()))
        return els[CurrentPin]->GetCurrentUndoName();
    return "";
}

string VersionManager::GetCurrentRedoName(){
    if(CurrentPin+1>=els.size()) return "";
    int fromPos=CurrentPin;
    for(int k=fromPos+1;k<=els.size();k++)
    {
        LONG Times=els[CurrentPin+1]->GetUsableRedoTimes();
        if(Times>0)
        {
            break;
        }
        else
        {
            els.erase(begin(els)+CurrentPin+1);
        }
    }
    if((CurrentPin+1>=0)&&(CurrentPin+1<els.size()))
        return els[CurrentPin+1]->GetCurrentRedoName();
    return "";
}

AutoPtr<Version>VersionManager::GetCurrentUndoVersion(){
    int fromPos=CurrentPin;
    for(int k=fromPos;k>=0;k--)
    {
        LONG Times=els[k]->GetUsableUndoTimes();
        if(Times>=0)
        {
            break;
        }
        else
        {
            els.erase(begin(els)+k);
            CurrentPin--;
        }
    }
    if((CurrentPin>=0)&&(CurrentPin<els.size()))
        return els[CurrentPin];
    return nullptr;
}
AutoPtr<Version>VersionManager::GetCurrentRedoVersion(){
    int fromPos=CurrentPin;
    for(int k=fromPos;k<els.size()-1;k++)
    {
        LONG Times=els[CurrentPin+1]->GetUsableRedoTimes();
        if(Times>0)
        {
            break;
        }
        else
        {
            els.erase(begin(els)+CurrentPin+1);
        }
    }
    if((CurrentPin+1>=0)&&(CurrentPin+1<els.size()))
        return els[CurrentPin+1];
    return nullptr;
}
}
