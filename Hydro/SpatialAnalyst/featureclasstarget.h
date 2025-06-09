#ifndef FEATURECLASSTARGET_H
#define FEATURECLASSTARGET_H
#include "samodel.h"
#include "Geometry/Point.h"
#include "Geometry/geometry2d.h"
#include "Datasources/shapfilefeatureclass.h"
#include "Datasources/memfeatureclass.h"
namespace SGIS{
class SGIS_EXPORT FeatureClassTarget
{
public:
    FeatureClassTarget();
    virtual~FeatureClassTarget();
    virtual AutoPtr<FeatureClass>CreateFeatureClass(SpatialReference*psp,VectorShapeType shpeType,TableDesc*desc)=0;
    virtual AutoPtr<FeatureClass>Create3DFeatureClass(SpatialReference*psp,VectorShapeType shpeType,TableDesc*desc)=0;
};

class SGIS_EXPORT ShapefileTarget :public FeatureClassTarget{
public:
    ShapefileTarget();
    virtual~ShapefileTarget();
    string&PathName();
    AutoPtr<FeatureClass>CreateFeatureClass(SpatialReference*psp,VectorShapeType shpeType,TableDesc*desc);
    AutoPtr<FeatureClass>Create3DFeatureClass(SpatialReference*psp,VectorShapeType shpeType,TableDesc*desc);
protected:
    string pathName;
};

class SGIS_EXPORT MemFeatureClassTarget :public FeatureClassTarget{
public:
    MemFeatureClassTarget();
    virtual~MemFeatureClassTarget();
    string&Name();
    AutoPtr<FeatureClass>CreateFeatureClass(SpatialReference*psp,VectorShapeType shpeType,TableDesc*desc);
    AutoPtr<FeatureClass>Create3DFeatureClass(SpatialReference*psp,VectorShapeType shpeType,TableDesc*desc);
protected:
    string name;
};


};


#endif // FEATURECLASSTARGET_H
