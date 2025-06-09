#ifndef FEATURECURSOR_H
#define FEATURECURSOR_H
#include "Base/autoptr.h"
#include "Base/variant.h"
#include "Geometry/geometry.h"
#include "tabledesc.h"
#include "Geometry/spatialreference.h"

namespace SGIS{


class SGIS_EXPORT Feature{
public:
    virtual~Feature();
    AutoPtr<Geometry>GetGeometry();
    Variant GetFieldValue(int fieldIndex);
    string GetFieldValueAsString(int fieldIndex);
    double GetFieldValueAsDouble(int fieldIndex);
    LONG GetFeatureID();
protected:
    Feature();
    friend class FeatureCursorCreator;
protected:
    AutoPtr<Geometry>geo;
    vector<Variant>values;
    LONG featureID;
};

class SGIS_EXPORT FeatureCursor{
public:
    virtual~FeatureCursor();
    int GetFeatureID(int recordIndex);
    int GetFeatureCount();
    AutoPtr<Geometry>GetFeature(int recordIndex);
    Variant GetFieldValue(int recordIndex,int FieldIndex);
    double GetFieldValueAsDouble(int recordIndex,int FieldIndex);
    string GetFieldValueAsString(int recordIndex,int FieldIndex);
    AutoPtr<TableDesc>GetTableDesc();
    AutoPtr<Feature>GetItem(int recordIndex);
    AutoPtr<SpatialReference>GetSpatialReference();
protected:
    FeatureCursor();
    friend class FeatureCursorCreator;
protected:
    vector<AutoPtr<Feature>>fs;
    AutoPtr<TableDesc>desc;
    AutoPtr<SpatialReference>psp;
};

class SGIS_EXPORT FeatureCursorCreator{
public:
    FeatureCursorCreator(AutoPtr<TableDesc>desc,AutoPtr<SpatialReference>psp);
    virtual~FeatureCursorCreator();
    void AddFeature(int featureID,AutoPtr<Geometry>geo,const vector<Variant>&values);
    AutoPtr<FeatureCursor>CreateFeatureCursor();
protected:
    AutoPtr<FeatureCursor>cursor;
};


}

#endif // FEATURECURSOR_H
