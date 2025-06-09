#ifndef HYDROANALYST_H
#define HYDROANALYST_H
#include "Base/autoptr.h"
#include "samodel.h"
#include "rastertarget.h"
#include "featureclasstarget.h"
#include "Geometry/geometry3d.h"
#include "imagelump.h"
//#include "reclassify.h"

namespace SGIS{

class SGIS_EXPORT HydroAnalyst :public SAModel
{
public:
    HydroAnalyst();
    virtual~HydroAnalyst();
    virtual string GetCatagory();
    FLOAT&OutputNoData();
protected:
    FLOAT nodata;
};

class SGIS_EXPORT HydroDemFill :public HydroAnalyst
{
public:
    HydroDemFill();
    virtual~HydroDemFill();
    string GetTypeName();
    AutoPtr<RasterBand>Execute(RasterBand*demBand,RasterTarget*target,CallBack*callBack=nullptr,Semaphora*signal=nullptr);
protected:
    void FillCell(AutoArray<Point2D<int>>&pointArray,FourValueArray&fvArray,FLOAT*pData,int Cols,int Rows, float&fillDem);
    void FillArea(AutoArray<Point2D<int>>&pointArray,FourValueArray&fvArray,FLOAT*pData,int Cols,int Rows, float fillDem);
};

class SGIS_EXPORT HydroLake :public HydroAnalyst
{
public:
    HydroLake();
    virtual~HydroLake();
    string GetTypeName();
    AutoPtr<RasterBand>Execute(RasterBand*fillBand,int lakeMinRadius,bool demFill,RasterTarget*target,CallBack*callBack=nullptr,Semaphora*signal=nullptr);
};

class SGIS_EXPORT HydroLakeTable :public HydroAnalyst
{
public:
    HydroLakeTable();
    virtual~HydroLakeTable();
    string GetTypeName();
    AutoPtr<DataTable>Execute(RasterBand*demBand,RasterBand*fillBand,int lakeMinRadius,CallBack*callBack=nullptr,Semaphora*signal=nullptr);
};

class SGIS_EXPORT HydroFlowDirection :public HydroAnalyst
{
public:
    HydroFlowDirection();
    virtual~HydroFlowDirection();
    string GetTypeName();
    AutoPtr<RasterBand>Execute(RasterBand*fillBand,RasterTarget*target,CallBack*callBack=nullptr,Semaphora*signal=nullptr);
    AutoPtr<RasterBand>Execute(RasterBand*demBand,RasterBand*fillBand,RasterTarget*target,CallBack*callBack=nullptr,Semaphora*signal=nullptr);
protected:
    float GetDirection(float*fVs,int x,int y);
    bool ComputeFlatDir(int x,int y,FourValueArray&fvArray,FLOAT*pData,FLOAT*pDatar);
    bool ComputeFlatDir2(int x,int y,FourValueArray&fvArray,FLOAT*pData,FLOAT*pDatar,FLOAT*oldData);
protected:
    LONG Cols;
    LONG Rows;
    BYTE Code[8];
    BYTE Code2[8];
};

class SGIS_EXPORT HydroFlowAccumulation :public HydroAnalyst
{
public:
    HydroFlowAccumulation();
    virtual~HydroFlowAccumulation();
    string GetTypeName();
    AutoPtr<RasterBand>Execute(RasterBand*fldBand,RasterBand*weightBand,RasterTarget*target,CallBack*callBack=nullptr,Semaphora*signal=nullptr);
};

class SGIS_EXPORT HydroFlowLength :public HydroAnalyst
{
public:
    HydroFlowLength();
    virtual~HydroFlowLength();
    string GetTypeName();
    AutoPtr<RasterBand>Execute(RasterBand*fldBand,RasterBand*weightBand,bool upsteam,RasterTarget*target,CallBack*callBack=nullptr,Semaphora*signal=nullptr);
protected:
    AutoPtr<RasterBand>FlowLengthUpstream(RasterBand*fldBand,RasterBand*weightBand,RasterTarget*target,CallBack*callBack=nullptr,Semaphora*signal=nullptr);
    AutoPtr<RasterBand>FlowLengthDownstream(RasterBand*fldBand,RasterBand*weightBand,RasterTarget*target,CallBack*callBack=nullptr,Semaphora*signal=nullptr);
};

class SGIS_EXPORT PourLineRoute{
public:
    PourLineRoute();
    ~PourLineRoute();
    void Add(int x,int y);
    void SetBegin(int x,int y);
    void LabelState(int state);
    int GetState();
    void CopyFrom(PourLineRoute*other);
    int GetSize();
    Point2D<int>GetNode(int nIndex);
    Point2D<int>GetBegin();
protected:
    vector<Point2D<int>>routes;
    int state;
    int beginX;
    int beginY;
};

class SGIS_EXPORT HydroLakePouringLine :public HydroAnalyst
{
public:
    HydroLakePouringLine();
    virtual~HydroLakePouringLine();
    string GetTypeName();
    AutoPtr<FeatureClass>Execute(RasterBand*demBand,RasterBand*fillBand,RasterBand*fldBand,RasterBand*accuBand,int lakeMinRadius,FeatureClassTarget*target,CallBack*callBack=nullptr,Semaphora*signal=nullptr);
protected:
    vector<AutoArray<Point2D<int>>>ClearUpPond(ImageLumpBlock&imageLumpBlock2);
    bool ComputeUpPond(ImageLumpBlock&imageLumpBlock,ImageLumpBlock&imageLumpBlock2,FLOAT*oldData,int lakeMinRadius,float&innerHeight);
    void DigDem(FLOAT*oldData,vector<Point2D<int>>&route,float innerHeight,int Cols,int Rows);
    void UpdateLocalHydro(FLOAT*fillData,FLOAT*oldData,ImageLumpBlock&imageLumpBlock,BYTE*flowDir);
    void ExtractPouringLine(AutoArray<Point2D<int>>&pointArray,FourValueArray&fvArray,FLOAT*pData,FLOAT*oldData,BYTE*sixArray,int Cols,int Rows,float innerHeight,ImageLumpBlock&imageLumpBlock,ImageLumpBlock&imageLumpBlock2,AutoPtr<FeatureClassEdit>pEdit);
    void PersuitSwitch(AutoPtr<PourLineRoute>route,ImageLumpBlock&imageLumpBlock,BYTE*maskBuffer);
    vector<AutoPtr<PourLineRoute>>PersuitSwitchs(AutoPtr<PourLineRoute>route,ImageLumpBlock&imageLumpBlock,BYTE*maskBuffer,FLOAT*oldData);
    AutoPtr<PourLineRoute>PersuitRoute(ImageLumpBlock&imageLumpBlock,BYTE*maskBuffer,FLOAT*oldData,int bX,int bY);
};

class SGIS_EXPORT HydroDig :public HydroAnalyst
{
public:
    HydroDig();
    virtual~HydroDig();
    string GetTypeName();
    AutoPtr<RasterBand>Execute(RasterBand*demBand,FeatureClass*pouringLine,RasterTarget*target,CallBack*callBack=nullptr,Semaphora*signal=nullptr);
    AutoPtr<RasterBand>Execute(RasterBand*demBand,RasterBand*fillBand,RasterBand*fldBand,FeatureClass*pouringLine,RasterTarget*target,CallBack*callBack=nullptr,Semaphora*signal=nullptr);
protected:
    void DigFeature(Polyline*polyline,float*pData,float*resultData,float digh);
    void BreachingHeading(int spillX,int spillY,int Cols,int Rows,FLOAT*oldData,BYTE*flowDir);
    void BreachingSink(AutoArray<Point2D<int>>&pointArray,FLOAT*oldData,BYTE*flowDir,ImageLumpBlock&imageLumpBlock);
};

class SGIS_EXPORT HydroLakePouringLineDig :public HydroAnalyst
{
public:
    HydroLakePouringLineDig();
    virtual~HydroLakePouringLineDig();
    string GetTypeName();
    AutoPtr<RasterBand>Execute(RasterBand*demBand,int lakeMinRadius,bool breachingHeader,int iters,RasterTarget*target,CallBack*callBack=nullptr,Semaphora*signal=nullptr);
protected:
    vector<AutoArray<Point2D<int>>>ClearUpPond(ImageLumpBlock&imageLumpBlock2);
    bool ComputeUpPond(ImageLumpBlock&imageLumpBlock,ImageLumpBlock&imageLumpBlock2,FLOAT*oldData,int lakeMinRadius,float&innerHeight);
    void BreachingHeading(int spillX,int spillY,int Cols,int Rows,FLOAT*oldData,BYTE*flowDir);
    void BreachingSink(AutoArray<Point2D<int>>&pointArray,FLOAT*oldData,BYTE*flowDir,ImageLumpBlock&imageLumpBlock);
    void DigDem(FLOAT*oldData,vector<Point2D<int>>&route,float innerHeight,int Cols,int Rows);
    void UpdateLocalHydro(FLOAT*fillData,FLOAT*oldData,ImageLumpBlock&imageLumpBlock,BYTE*flowDir);
    void DigByPouringLine(AutoArray<Point2D<int>>&pointArray,FourValueArray&fvArray,FLOAT*pData,FLOAT*oldData,BYTE*sixArray,int Cols,int Rows,float innerHeight,ImageLumpBlock&imageLumpBlock,ImageLumpBlock&imageLumpBlock2);
    void PersuitSwitch(AutoPtr<PourLineRoute>route,ImageLumpBlock&imageLumpBlock,BYTE*maskBuffer);
    vector<AutoPtr<PourLineRoute>>PersuitSwitchs(AutoPtr<PourLineRoute>route,ImageLumpBlock&imageLumpBlock,BYTE*maskBuffer,FLOAT*oldData);
    AutoPtr<PourLineRoute>PersuitRoute(ImageLumpBlock&imageLumpBlock,BYTE*maskBuffer,FLOAT*oldData,int bX,int bY);
    AutoPtr<RasterBand>innerExecute(RasterBand*demBand,RasterBand*fillBand,RasterBand*fldBand,RasterBand*accuBand,int lakeMinRadius,bool breachingHeader,RasterTarget*target,CallBack*callBack=nullptr,Semaphora*signal=nullptr);
};


};
#endif // HYDROANALYST_H
