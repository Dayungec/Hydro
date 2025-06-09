#ifndef FEATURECLASSTOOL_H
#define FEATURECLASSTOOL_H

#include "Base/autoptr.h"
#include "Geometry/geometrybordercapture.h"
#include "dataset.h"

namespace SGIS{

class SGIS_EXPORT FeatureClassTool
{
public:
    FeatureClassTool(FeatureClass*pfc);
    virtual~FeatureClassTool();
public:
    static int FindFeatureID(FeatureClass*pfc,int FeatureID);
    AutoPtr<FeatureCursor>Select(FeatureClass*pfc,QueryFilter*pFilter,CallBack*callBack=nullptr);
    AutoPtr<FeatureCursor>SelectByPoint(FeatureClass*pfc,Point*centerPoint,double toler,CallBack*callBack=nullptr);
    AutoPtr<FeatureCursor>SelectByPointEx(FeatureClass*pfc,Point*centerPoint,double toler,const vector<string>&fields,CallBack*callBack=nullptr);
    bool CalculateField(string field,string expression,CallBack*callBack=nullptr);
    static bool Query(FeatureClass*pfc,QueryFilter*pFilter,SelectionSelMode mode,CallBack*callBack=nullptr);
    static void QueryByPoint(FeatureClass*pfc,Point*centerPoint,double toler,SelectionSelMode mode,CallBack*callBack=nullptr);
public:
    int FindFeatureID(int FeatureID);
    AutoPtr<IndexedArray<string>>GetUniqueValues(vector<int>fieldIndices,string fieldSplit,int maxSize);
    AutoPtr<Histogram>ComputeHistogram(string fieldName,DOUBLE minValue,DOUBLE maxValue,LONG ColumnsNum);
    bool ComputeStatistics(string fieldName,DOUBLE*minValue,DOUBLE*maxValue,DOUBLE*meanValue,DOUBLE*stdValue);
    AutoPtr<FeatureCursor>Select(QueryFilter*pFilter,CallBack*callBack=nullptr);
    AutoPtr<FeatureCursor>SelectByPoint(Point*centerPoint,double toler,CallBack*callBack=nullptr);
    AutoPtr<FeatureCursor>SelectByPointEx(Point*centerPoint,double toler,const vector<string>&fields,CallBack*callBack=nullptr);
    bool Query(QueryFilter*pFilter,SelectionSelMode mode,CallBack*callBack=nullptr);
    void QueryByPoint(Point*centerPoint,double toler,SelectionSelMode mode,CallBack*callBack=nullptr);
    int CaptureNearestFeatureID(Point*centerPoint,double toler,SelectionSet*filter,DOUBLE*minDist);
    bool TestCaptureNearestFeature(Point*centerPoint,double toler,SelectionSet*filter);
    vector<GeometryBorderCaptureData>CaptureFeaturesVertex(Point*centerPoint,double toler,SelectionSet*filter,bool needCaptureSeg);
protected:
    FeatureClass*pfc;
};

}
#endif // FEATURECLASSTOOL_H
