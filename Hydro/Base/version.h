#ifndef VERSION_H
#define VERSION_H
#include "base_globle.h"
#include "autoptr.h"
#include "variant.h"
#include "eventhelper.h"

namespace SGIS{

class VersionManager;

class SGIS_EXPORT Version
{
public:
    Version(){};
    virtual~Version(){};
    virtual int GetHistoryMaxItems()=0;
    virtual void SetHistoryMaxItems(int maxItems)=0;
    virtual int GetUsableUndoTimes()=0;
    virtual int GetUsableRedoTimes()=0;
    virtual bool UndoEdit()=0;
    virtual bool RedoEdit()=0;
    virtual bool BeginNewVersion(string name,bool EditChanged=true)=0;
    virtual bool GetCurrentEditChanged()=0;
    virtual string GetCurrentUndoName()=0;
    virtual string GetCurrentRedoName()=0;
    virtual bool SetCurrentUndoName(string name)=0;
    virtual bool SetCurrentRedoName(string name)=0;
    virtual bool ClipForewardAllRedos()=0;
    virtual bool RemoveOneBackwardUndo()=0;
    virtual VersionManager* GetVersionManager()=0;
    virtual void SetVersionManager(VersionManager*pManager)=0;
};

class SGIS_EXPORT VersionManager{
 public:
    VersionManager();
    virtual~VersionManager();
    int GetVersionCount();
    AutoPtr<Version>GetVersionItem(int nIndex);
    int GetHistoryMaxItems();
    void SetHistoryMaxItems(int maxItems);
    void  RegisterVersion(AutoPtr<Version>item);
    bool FindOldVersion(AutoPtr<Version>item);
    void EmptyVersions();
    void RemoveVersion(AutoPtr<Version>pVersion);
    int GetUsableUndoTimes();
    int GetUsableRedoTimes();
    bool UndoEdit();
    bool RedoEdit();
    string GetCurrentUndoName();
    string GetCurrentRedoName();
    AutoPtr<Version>GetCurrentUndoVersion();
    AutoPtr<Version>GetCurrentRedoVersion();
public:
    Event VersionChanged;
protected:
    vector<AutoPtr<Version>>els;
    LONG HistoryMaxItems;
    LONG CurrentPin;
};

}

#endif // VERSION_H
