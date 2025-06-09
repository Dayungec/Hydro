#ifndef SELECTIONSET_H
#define SELECTIONSET_H
#include "Base/BaseArray.h"
#include "Base/autoptr.h"


namespace SGIS{

class SGIS_EXPORT SelectionSet
{
public:
    SelectionSet();
    virtual~SelectionSet();
    int GetSize();
    int GetFeatureID(int nIndex);
    bool AddFeatureID(int fid);
    bool RemoveFeatureID(int fid);
    void Remove(int nIndex);
    void Clear();
    bool FindFeatureID(int fid);
    int FindFeatureIndex(int fid);
    AutoPtr<SelectionSet>Clone();
    void CopyFromOther(SelectionSet*other);
    void Intersect(SelectionSet*other);
protected:
    OrderArray<int>pArray;
};

}

#endif // SELECTIONSET_H
