#ifndef QUERYFILTER_H
#define QUERYFILTER_H
#include "Base/autoptr.h"
#include "Base/BaseArray.h"
#include "Geometry/geometrybase.h"

namespace SGIS{
class SGIS_EXPORT QueryFilter
{
public:
    QueryFilter();
    virtual~QueryFilter();
    string&WhereClause();
    string GetSubFields();
    void SetSubFields(string fields);
    bool AddField(string name);
    int GetFieldCount();
    string GetField(int nIndex);
protected:
    IndexedArray<string>subFields;
    string whereClause;
};

class SGIS_EXPORT SpatialFilter:
        public QueryFilter
{
public:
    SpatialFilter();
    ~SpatialFilter();
    GeometryTopoType&SpatialRel();
    AutoPtr<Geometry>GetGeometry();
    void SetGeometry(AutoPtr<Geometry>geo);
protected:
    GeometryTopoType gType;
    AutoPtr<Geometry>refGeo;
};

}

#endif // QUERYFILTER_H
